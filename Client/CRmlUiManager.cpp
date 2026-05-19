#include <SAMP+/client/CRmlUiManager.h>

#include <SAMP+/OMPPlusProtocol.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/Proxy/CDInput8DeviceProxy.h>

#include <DirectX/dinput.h>
#include <DirectX/d3dx9.h>
#include <windowsx.h>

#include <algorithm>
#include <cstring>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

namespace
{
	struct UiVertex
	{
		float x;
		float y;
		float z;
		float rhw;
		D3DCOLOR color;
	};

	struct InventorySlot
	{
		bool used;
		uint32_t itemId;
		uint16_t amount;
		std::string label;
		std::string description;
		std::string icon;
		std::vector<std::pair<std::string, std::string> > actions;

		InventorySlot()
			: used(false)
			, itemId(0)
			, amount(0)
		{
		}
	};

	struct UiDocument
	{
		std::string id;
		uint8_t templateId;
		uint32_t flags;
		uint16_t capacity;
		std::string title;
		std::string body;
		std::map<std::string, std::string> data;
		std::vector<InventorySlot> slots;
		int hoveredSlot;
		RECT bounds;

		UiDocument()
			: templateId(OMPPlusProtocol::UiTemplatePanel)
			, flags(0)
			, capacity(0)
			, hoveredSlot(-1)
		{
			SetRectEmpty(&bounds);
		}
	};

	struct InventoryLayout
	{
		int slotSize;
		int gap;
		int columns;
		int maxRows;
		int startX;
		int startY;
	};

	struct InventoryDragState
	{
		bool candidate;
		bool active;
		std::string documentId;
		int sourceSlot;
		POINT startMouse;
		InventorySlot item;

		InventoryDragState()
			: candidate(false)
			, active(false)
			, sourceSlot(-1)
		{
			startMouse.x = 0;
			startMouse.y = 0;
		}
	};

	struct InventoryContextMenuState
	{
		bool active;
		std::string documentId;
		int slot;
		int x;
		int y;
		int hoveredAction;
		std::vector<std::pair<std::string, std::string> > actions;

		InventoryContextMenuState()
			: active(false)
			, slot(-1)
			, x(0)
			, y(0)
			, hoveredAction(-1)
		{
		}
	};

	struct InventorySplitState
	{
		bool active;
		std::string documentId;
		int slot;
		int amount;
		int maxAmount;
		std::string actionId;

		InventorySplitState()
			: active(false)
			, slot(-1)
			, amount(1)
			, maxAmount(1)
		{
		}
	};

	const DWORD UiFvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
	const size_t MaxDocumentIdLength = 31;
	const size_t MaxTitleLength = 64;
	const size_t MaxBodyLength = 255;
	const size_t MaxKeyLength = 31;
	const size_t MaxValueLength = 255;
	const size_t MaxSlotLabelLength = 48;
	const size_t MaxSlotDescriptionLength = 96;
	const size_t MaxIconLength = 48;
	const size_t MaxActionsLength = 255;
	const uint16_t MaxInventorySlots = 120;

	static HWND g_window = NULL;
	static IDirect3DDevice9* g_device = NULL;
	static ID3DXFont* g_font = NULL;
	static ID3DXFont* g_titleFont = NULL;
	static std::vector<UiDocument> g_documents;
	static POINT g_mouse = { 0, 0 };
	static InventoryDragState g_drag;
	static InventoryContextMenuState g_contextMenu;
	static InventorySplitState g_split;
	static bool g_initialized = false;
	static bool g_loggedFallback = false;
	static bool g_captureActive = false;

	void ClampMouseToWindow()
	{
		if (!g_window)
			return;

		RECT client = {};
		if (!GetClientRect(g_window, &client))
			return;

		const LONG width = client.right - client.left;
		const LONG height = client.bottom - client.top;
		if (width <= 1 || height <= 1)
			return;

		if (g_mouse.x < 0)
			g_mouse.x = 0;
		else if (g_mouse.x >= width)
			g_mouse.x = width - 1;

		if (g_mouse.y < 0)
			g_mouse.y = 0;
		else if (g_mouse.y >= height)
			g_mouse.y = height - 1;
	}

	D3DCOLOR C(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
	{
		return D3DCOLOR_ARGB(a, r, g, b);
	}

	bool PointInRect(int x, int y, const RECT& rect)
	{
		return x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom;
	}

	void UpdateMouseFromCursor()
	{
		if (!g_window)
			return;

		POINT point = {};
		if (GetCursorPos(&point) && ScreenToClient(g_window, &point))
		{
			g_mouse = point;
			ClampMouseToWindow();
		}
	}

	bool ReadBoundString(RakNet::BitStream& stream, std::string& value, size_t maxLength)
	{
		uint8_t length = 0;
		if (!stream.Read(length) || length > maxLength)
			return false;

		char buffer[256] = {};
		if (length && !stream.Read(buffer, length))
			return false;

		value.assign(buffer, length);
		return true;
	}

	void WriteBoundString(RakNet::BitStream& stream, const std::string& value, size_t maxLength)
	{
		const size_t boundedLength = value.length() < maxLength ? value.length() : maxLength;
		stream.Write(static_cast<uint8_t>(boundedLength));
		if (boundedLength)
			stream.Write(value.c_str(), static_cast<int>(boundedLength));
	}

	std::vector<UiDocument>::iterator FindDocument(const std::string& id)
	{
		return std::find_if(g_documents.begin(), g_documents.end(), [&id](const UiDocument& document)
		{
			return document.id == id;
		});
	}

	UiDocument* TopDocument()
	{
		if (g_documents.empty())
			return NULL;
		return &g_documents.back();
	}

	bool IsInteractive(const UiDocument& document)
	{
		return (document.flags & (OMPPlusProtocol::UiFlagCaptureMouse | OMPPlusProtocol::UiFlagModal)) != 0;
	}

	bool IsInventoryTemplate(const UiDocument& document)
	{
		return document.templateId == OMPPlusProtocol::UiTemplateInventory || document.templateId == OMPPlusProtocol::UiTemplateStorage;
	}

	bool IsSlotUsed(const UiDocument& document, int slot)
	{
		return slot >= 0 && slot < static_cast<int>(document.slots.size()) && document.slots[slot].used;
	}

	InventoryLayout GetInventoryLayout(const UiDocument& document)
	{
		InventoryLayout layout;
		layout.slotSize = 62;
		layout.gap = 8;
		layout.columns = 6;
		layout.maxRows = 5;
		layout.startX = document.bounds.left + 20;
		layout.startY = document.bounds.top + 104;
		return layout;
	}

	RECT GetInventorySlotRect(const UiDocument& document, uint16_t slot)
	{
		InventoryLayout layout = GetInventoryLayout(document);
		const int col = slot % layout.columns;
		const int row = slot / layout.columns;
		const int sx = layout.startX + col * (layout.slotSize + layout.gap);
		const int sy = layout.startY + row * (layout.slotSize + layout.gap);
		RECT rect = { sx, sy, sx + layout.slotSize, sy + layout.slotSize };
		return rect;
	}

	int FindInventorySlotAt(UiDocument& document, int x, int y)
	{
		const uint16_t requestedCapacity = document.capacity ? document.capacity : 30;
		const uint16_t capacity = requestedCapacity < MaxInventorySlots ? requestedCapacity : MaxInventorySlots;
		if (document.slots.size() < capacity)
			document.slots.resize(capacity);

		const int visibleSlots = 6 * 5;
		for (uint16_t slot = 0; slot < capacity && slot < visibleSlots; ++slot)
		{
			if (PointInRect(x, y, GetInventorySlotRect(document, slot)))
				return slot;
		}

		return -1;
	}

	void UpdateInventoryHover(UiDocument& document)
	{
		document.hoveredSlot = IsInventoryTemplate(document) ? FindInventorySlotAt(document, g_mouse.x, g_mouse.y) : -1;
	}

	void ClearInventoryDrag()
	{
		g_drag = InventoryDragState();
	}

	void ClearInventoryContextMenu()
	{
		g_contextMenu = InventoryContextMenuState();
	}

	void ClearInventorySplit()
	{
		g_split = InventorySplitState();
	}

	std::string Trim(const std::string& value)
	{
		size_t start = 0;
		while (start < value.length() && (value[start] == ' ' || value[start] == '\t'))
			++start;

		size_t end = value.length();
		while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t'))
			--end;

		return value.substr(start, end - start);
	}

	std::vector<std::pair<std::string, std::string> > ParseInventoryActions(const std::string& encoded)
	{
		std::vector<std::pair<std::string, std::string> > actions;
		size_t cursor = 0;
		while (cursor < encoded.length() && actions.size() < 10)
		{
			size_t next = encoded.find(';', cursor);
			if (next == std::string::npos)
				next = encoded.length();

			std::string token = Trim(encoded.substr(cursor, next - cursor));
			if (!token.empty())
			{
				size_t sep = token.find(':');
				std::string id = sep == std::string::npos ? token : token.substr(0, sep);
				std::string label = sep == std::string::npos ? token : token.substr(sep + 1);
				id = Trim(id);
				label = Trim(label);
				if (!id.empty())
				{
					if (label.empty())
						label = id;
					actions.push_back(std::make_pair(id, label));
				}
			}

			cursor = next + 1;
		}
		return actions;
	}

	std::vector<std::pair<std::string, std::string> > GetInventoryActions(const InventorySlot& slot)
	{
		if (!slot.actions.empty())
			return slot.actions;

		std::vector<std::pair<std::string, std::string> > actions;
		actions.push_back(std::make_pair(std::string("use"), std::string("Use")));
		if (slot.amount > 1)
			actions.push_back(std::make_pair(std::string("split"), std::string("Split Stack")));
		actions.push_back(std::make_pair(std::string("drop"), std::string("Drop")));
		actions.push_back(std::make_pair(std::string("inspect"), std::string("Inspect")));
		return actions;
	}

	std::string BuildInventoryActionPayload(const InventorySlot& slot, const std::string& actionId)
	{
		char prefix[96] = {};
		sprintf(prefix, "%s|%u|%u|", actionId.c_str(), static_cast<unsigned>(slot.itemId), static_cast<unsigned>(slot.amount));
		std::string payload(prefix);
		payload += slot.label;
		return payload;
	}

	std::string BuildInventorySplitPayload(const InventorySlot& slot, int amount, const std::string& actionId)
	{
		char prefix[96] = {};
		sprintf(prefix, "%d|%s|%u|%u|", amount, actionId.c_str(), static_cast<unsigned>(slot.itemId), static_cast<unsigned>(slot.amount));
		std::string payload(prefix);
		payload += slot.label;
		return payload;
	}

	void OpenInventoryContextMenu(UiDocument& document)
	{
		UpdateInventoryHover(document);
		if (!IsSlotUsed(document, document.hoveredSlot))
		{
			ClearInventoryContextMenu();
			return;
		}

		g_contextMenu = InventoryContextMenuState();
		g_contextMenu.active = true;
		g_contextMenu.documentId = document.id;
		g_contextMenu.slot = document.hoveredSlot;
		g_contextMenu.x = g_mouse.x + 10;
		g_contextMenu.y = g_mouse.y + 10;
		g_contextMenu.actions = GetInventoryActions(document.slots[document.hoveredSlot]);
	}

	void BeginInventorySplit(UiDocument& document, int slot, const std::string& actionId)
	{
		if (!IsSlotUsed(document, slot) || document.slots[slot].amount <= 1)
			return;

		g_split = InventorySplitState();
		g_split.active = true;
		g_split.documentId = document.id;
		g_split.slot = slot;
		g_split.maxAmount = document.slots[slot].amount - 1;
		if (g_split.maxAmount < 1)
			g_split.maxAmount = 1;
		g_split.amount = (g_split.maxAmount + 1) / 2;
		if (g_split.amount < 1)
			g_split.amount = 1;
		g_split.actionId = actionId;
	}

	bool ShouldCloseOnEscape(const UiDocument& document)
	{
		return (document.flags & OMPPlusProtocol::UiFlagCloseOnEscape) != 0;
	}

	void SendUiEvent(const UiDocument& document, uint8_t eventType, uint16_t slot, const std::string& element, const std::string& payload)
	{
		RakNet::BitStream stream;
		WriteBoundString(stream, document.id, MaxDocumentIdLength);
		stream.Write(eventType);
		stream.Write(slot);
		WriteBoundString(stream, element, MaxKeyLength);
		WriteBoundString(stream, payload, MaxValueLength);
		Network::SendRPC(OMPPlusProtocol::ON_UI_EVENT, &stream);
	}

	std::string BuildInventoryDropPayload(int targetSlot, const InventorySlot& slot)
	{
		char prefix[64] = {};
		sprintf(prefix, "%d|%u|%u|", targetSlot, static_cast<unsigned>(slot.itemId), static_cast<unsigned>(slot.amount));
		std::string payload(prefix);
		payload += slot.label;
		return payload;
	}

	bool BeginInventoryDrag(UiDocument& document)
	{
		UpdateInventoryHover(document);
		if (!IsSlotUsed(document, document.hoveredSlot))
			return false;

		g_drag = InventoryDragState();
		g_drag.candidate = true;
		g_drag.documentId = document.id;
		g_drag.sourceSlot = document.hoveredSlot;
		g_drag.startMouse = g_mouse;
		g_drag.item = document.slots[document.hoveredSlot];
		return true;
	}

	void UpdateInventoryDrag()
	{
		if (!g_drag.candidate || g_drag.active)
			return;

		const int dx = g_mouse.x - g_drag.startMouse.x;
		const int dy = g_mouse.y - g_drag.startMouse.y;
		if ((dx * dx + dy * dy) >= 25)
			g_drag.active = true;
	}

	bool FinishInventoryDrag(UiDocument& document)
	{
		if (!g_drag.candidate || g_drag.documentId != document.id)
			return false;

		UpdateInventoryHover(document);
		if (g_drag.active)
		{
			if (document.hoveredSlot >= 0)
			{
				if (document.hoveredSlot != g_drag.sourceSlot)
				{
					SendUiEvent(
						document,
						OMPPlusProtocol::UiEventSlotDrop,
						static_cast<uint16_t>(g_drag.sourceSlot),
						"slot_drop",
						BuildInventoryDropPayload(document.hoveredSlot, g_drag.item)
					);
				}
			}
			else if (!PointInRect(g_mouse.x, g_mouse.y, document.bounds))
			{
				SendUiEvent(
					document,
					OMPPlusProtocol::UiEventWorldDrop,
					static_cast<uint16_t>(g_drag.sourceSlot),
					"world_drop",
					BuildInventoryDropPayload(-1, g_drag.item)
				);
			}

			ClearInventoryDrag();
			return true;
		}

		ClearInventoryDrag();
		return false;
	}

	void CloseDocument(std::vector<UiDocument>::iterator it, bool notify)
	{
		if (it == g_documents.end())
			return;

		if (notify)
			SendUiEvent(*it, OMPPlusProtocol::UiEventClose, 0, "document", "");

		if (g_drag.documentId == it->id)
			ClearInventoryDrag();
		if (g_contextMenu.documentId == it->id)
			ClearInventoryContextMenu();
		if (g_split.documentId == it->id)
			ClearInventorySplit();

		g_documents.erase(it);
		CDInput8DeviceProxy::RequestInputReset();
	}

	void DrawRect(IDirect3DDevice9* device, float x, float y, float width, float height, D3DCOLOR color)
	{
		if (!device || width <= 0.0f || height <= 0.0f)
			return;

		UiVertex vertices[4] =
		{
			{ x - 0.5f, y - 0.5f, 0.0f, 1.0f, color },
			{ x + width - 0.5f, y - 0.5f, 0.0f, 1.0f, color },
			{ x - 0.5f, y + height - 0.5f, 0.0f, 1.0f, color },
			{ x + width - 0.5f, y + height - 0.5f, 0.0f, 1.0f, color }
		};

		device->SetTexture(0, NULL);
		device->SetFVF(UiFvf);
		device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(UiVertex));
	}

	void DrawBorder(IDirect3DDevice9* device, float x, float y, float width, float height, D3DCOLOR color)
	{
		DrawRect(device, x, y, width, 1.0f, color);
		DrawRect(device, x, y + height - 1.0f, width, 1.0f, color);
		DrawRect(device, x, y, 1.0f, height, color);
		DrawRect(device, x + width - 1.0f, y, 1.0f, height, color);
	}

	void DrawTextLine(ID3DXFont* font, const std::string& text, int x, int y, int width, D3DCOLOR color, DWORD format = DT_LEFT | DT_TOP | DT_NOCLIP)
	{
		if (!font || text.empty())
			return;

		RECT rect = { x, y, x + width, y + 120 };
		font->DrawTextA(NULL, text.c_str(), -1, &rect, format, color);
	}

	void DrawWrappedText(ID3DXFont* font, const std::string& text, int x, int y, int width, int height, D3DCOLOR color)
	{
		if (!font || text.empty())
			return;

		RECT rect = { x, y, x + width, y + height };
		font->DrawTextA(NULL, text.c_str(), -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK, color);
	}

	void DrawCursor(IDirect3DDevice9* device)
	{
		const float x = static_cast<float>(g_mouse.x);
		const float y = static_cast<float>(g_mouse.y);
		DrawRect(device, x, y, 2.0f, 15.0f, C(255, 235, 235, 235));
		DrawRect(device, x + 2.0f, y + 2.0f, 2.0f, 10.0f, C(255, 235, 235, 235));
		DrawRect(device, x + 4.0f, y + 5.0f, 2.0f, 7.0f, C(255, 235, 235, 235));
		DrawRect(device, x + 6.0f, y + 8.0f, 2.0f, 5.0f, C(255, 235, 235, 235));
		DrawRect(device, x + 1.0f, y + 1.0f, 1.0f, 15.0f, C(180, 0, 0, 0));
	}

	bool EnsureFonts(IDirect3DDevice9* device)
	{
		if (!device)
			return false;

		if (g_font && g_titleFont)
			return true;

		if (!g_font)
		{
			if (FAILED(D3DXCreateFontA(device, 15, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &g_font)))
				return false;
		}

		if (!g_titleFont)
		{
			if (FAILED(D3DXCreateFontA(device, 18, 0, FW_SEMIBOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &g_titleFont)))
				return false;
		}

		return true;
	}

	void RenderPanelBase(IDirect3DDevice9* device, UiDocument& document, int x, int y, int width, int height)
	{
		document.bounds.left = x;
		document.bounds.top = y;
		document.bounds.right = x + width;
		document.bounds.bottom = y + height;

		DrawRect(device, static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height), C(232, 3, 4, 5));
		DrawRect(device, static_cast<float>(x + 1), static_cast<float>(y + 1), static_cast<float>(width - 2), 34.0f, C(215, 17, 19, 21));
		DrawBorder(device, static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height), C(170, 215, 218, 218));
		DrawRect(device, static_cast<float>(x + 14), static_cast<float>(y + 36), static_cast<float>(width - 28), 1.0f, C(90, 230, 230, 230));
		DrawTextLine(g_titleFont, document.title.empty() ? document.id : document.title, x + 16, y + 9, width - 32, C(255, 240, 245, 242));
	}

	void ClampPopupToWindow(int& x, int& y, int width, int height)
	{
		RECT client = {};
		if (!g_window || !GetClientRect(g_window, &client))
			return;

		if (x + width > client.right)
			x = client.right - width - 8;
		if (y + height > client.bottom)
			y = client.bottom - height - 8;
		if (x < 8)
			x = 8;
		if (y < 8)
			y = 8;
	}

	RECT GetContextMenuRect()
	{
		const int width = 210;
		const int height = 34 + static_cast<int>(g_contextMenu.actions.size()) * 30 + 10;
		int x = g_contextMenu.x;
		int y = g_contextMenu.y;
		ClampPopupToWindow(x, y, width, height);
		RECT rect = { x, y, x + width, y + height };
		return rect;
	}

	RECT GetSplitDialogRect(const UiDocument& document)
	{
		const int width = 340;
		const int height = 178;
		const int x = document.bounds.left + ((document.bounds.right - document.bounds.left) - width) / 2;
		const int y = document.bounds.top + ((document.bounds.bottom - document.bounds.top) - height) / 2;
		RECT rect = { x, y, x + width, y + height };
		return rect;
	}

	void RenderInventoryContextMenu(IDirect3DDevice9* device, UiDocument& document)
	{
		if (!g_contextMenu.active || g_contextMenu.documentId != document.id)
			return;

		if (!IsSlotUsed(document, g_contextMenu.slot))
		{
			ClearInventoryContextMenu();
			return;
		}

		RECT rect = GetContextMenuRect();
		const int width = rect.right - rect.left;
		const int rowH = 30;
		g_contextMenu.hoveredAction = -1;

		DrawRect(device, static_cast<float>(rect.left), static_cast<float>(rect.top), static_cast<float>(width), static_cast<float>(rect.bottom - rect.top), C(238, 5, 6, 7));
		DrawBorder(device, static_cast<float>(rect.left), static_cast<float>(rect.top), static_cast<float>(width), static_cast<float>(rect.bottom - rect.top), C(190, 220, 222, 222));
		DrawTextLine(g_titleFont, document.slots[g_contextMenu.slot].label, rect.left + 12, rect.top + 8, width - 24, C(255, 255, 255, 255));
		DrawRect(device, static_cast<float>(rect.left + 10), static_cast<float>(rect.top + 32), static_cast<float>(width - 20), 1.0f, C(90, 220, 220, 220));

		for (size_t i = 0; i < g_contextMenu.actions.size(); ++i)
		{
			const int y = rect.top + 38 + static_cast<int>(i) * rowH;
			RECT row = { rect.left + 8, y, rect.right - 8, y + rowH - 4 };
			const bool hovered = PointInRect(g_mouse.x, g_mouse.y, row);
			if (hovered)
				g_contextMenu.hoveredAction = static_cast<int>(i);

			DrawRect(device, static_cast<float>(row.left), static_cast<float>(row.top), static_cast<float>(row.right - row.left), static_cast<float>(row.bottom - row.top), hovered ? C(245, 82, 86, 89) : C(210, 11, 15, 17));
			DrawBorder(device, static_cast<float>(row.left), static_cast<float>(row.top), static_cast<float>(row.right - row.left), static_cast<float>(row.bottom - row.top), hovered ? C(230, 245, 245, 245) : C(70, 90, 98, 98));
			DrawTextLine(g_font, g_contextMenu.actions[i].second, row.left + 12, row.top + 6, row.right - row.left - 24, C(255, 255, 255, 255));
		}
	}

	void RenderInventorySplitDialog(IDirect3DDevice9* device, UiDocument& document)
	{
		if (!g_split.active || g_split.documentId != document.id || !IsSlotUsed(document, g_split.slot))
			return;

		RECT rect = GetSplitDialogRect(document);
		const int width = rect.right - rect.left;
		DrawRect(device, static_cast<float>(rect.left), static_cast<float>(rect.top), static_cast<float>(width), static_cast<float>(rect.bottom - rect.top), C(242, 4, 5, 6));
		DrawBorder(device, static_cast<float>(rect.left), static_cast<float>(rect.top), static_cast<float>(width), static_cast<float>(rect.bottom - rect.top), C(210, 245, 245, 245));
		DrawTextLine(g_titleFont, "Split Stack", rect.left + 14, rect.top + 10, width - 28, C(255, 255, 255, 255));
		DrawRect(device, static_cast<float>(rect.left + 12), static_cast<float>(rect.top + 40), static_cast<float>(width - 24), 1.0f, C(90, 230, 230, 230));

		const InventorySlot& slot = document.slots[g_split.slot];
		DrawTextLine(g_font, slot.label, rect.left + 16, rect.top + 55, width - 32, C(230, 225, 232, 232));
		char amountText[64] = {};
		sprintf(amountText, "%d / %d", g_split.amount, g_split.maxAmount);
		DrawTextLine(g_titleFont, amountText, rect.left + 16, rect.top + 82, width - 32, C(255, 255, 255, 255), DT_CENTER | DT_TOP | DT_NOCLIP);

		const char* labels[6] = { "-", "+", "Half", "Max", "Split", "Cancel" };
		for (int i = 0; i < 6; ++i)
		{
			const int buttonW = i < 2 ? 46 : 70;
			const int bx = rect.left + 16 + (i < 2 ? i * 54 : 108 + (i - 2) * 76);
			const int by = i < 4 ? rect.top + 114 : rect.top + 144;
			const int actualW = i < 4 ? buttonW : 145;
			const int actualX = i == 4 ? rect.left + 16 : (i == 5 ? rect.left + 179 : bx);
			RECT button = { actualX, by, actualX + actualW, by + 24 };
			const bool hovered = PointInRect(g_mouse.x, g_mouse.y, button);
			DrawRect(device, static_cast<float>(button.left), static_cast<float>(button.top), static_cast<float>(button.right - button.left), static_cast<float>(button.bottom - button.top), hovered ? C(245, 82, 86, 89) : C(215, 12, 16, 18));
			DrawBorder(device, static_cast<float>(button.left), static_cast<float>(button.top), static_cast<float>(button.right - button.left), static_cast<float>(button.bottom - button.top), hovered ? C(230, 245, 245, 245) : C(80, 95, 104, 104));
			DrawTextLine(g_font, labels[i], button.left, button.top + 5, button.right - button.left, C(255, 255, 255, 255), DT_CENTER | DT_TOP | DT_NOCLIP);
		}
	}

	void RenderInventory(IDirect3DDevice9* device, UiDocument& document, const D3DVIEWPORT9& viewport)
	{
		const int panelW = 780;
		const int panelH = 548;
		const int x = (static_cast<int>(viewport.Width) - panelW) / 2;
		const int y = (static_cast<int>(viewport.Height) - panelH) / 2;
		RenderPanelBase(device, document, x, y, panelW, panelH);

		DrawWrappedText(g_font, document.body, x + 18, y + 48, panelW - 36, 42, C(220, 230, 234, 232));

		const InventoryLayout layout = GetInventoryLayout(document);
		const int slotSize = layout.slotSize;
		const int gap = layout.gap;
		const int columns = layout.columns;
		const int startX = layout.startX;
		const int startY = layout.startY;
		const int maxRows = layout.maxRows;
		document.hoveredSlot = -1;

		const uint16_t requestedCapacity = document.capacity ? document.capacity : 30;
		const uint16_t capacity = requestedCapacity < MaxInventorySlots ? requestedCapacity : MaxInventorySlots;
		if (document.slots.size() < capacity)
			document.slots.resize(capacity);

		for (uint16_t i = 0; i < capacity && i < columns * maxRows; ++i)
		{
			const int col = i % columns;
			const int row = i / columns;
			const int sx = startX + col * (slotSize + gap);
			const int sy = startY + row * (slotSize + gap);
			RECT slotRect = { sx, sy, sx + slotSize, sy + slotSize };
			const bool hovered = PointInRect(g_mouse.x, g_mouse.y, slotRect);
			if (hovered)
				document.hoveredSlot = i;

			const bool dragSource = g_drag.candidate && g_drag.documentId == document.id && g_drag.sourceSlot == static_cast<int>(i);
			const bool dragTarget = g_drag.active && g_drag.documentId == document.id && document.hoveredSlot == static_cast<int>(i) && !dragSource;

			D3DCOLOR slotFill = hovered ? C(238, 68, 72, 76) : C(205, 11, 15, 17);
			D3DCOLOR slotBorder = hovered ? C(230, 245, 245, 245) : C(96, 95, 104, 104);
			if (dragSource)
			{
				slotFill = C(155, 45, 34, 12);
				slotBorder = C(245, 255, 171, 65);
			}
			else if (dragTarget)
			{
				slotFill = C(235, 91, 75, 38);
				slotBorder = C(255, 255, 194, 96);
			}

			DrawRect(device, static_cast<float>(sx), static_cast<float>(sy), static_cast<float>(slotSize), static_cast<float>(slotSize), slotFill);
			DrawBorder(device, static_cast<float>(sx), static_cast<float>(sy), static_cast<float>(slotSize), static_cast<float>(slotSize), slotBorder);

			const InventorySlot& slot = document.slots[i];
			if (slot.used)
			{
				DrawRect(device, static_cast<float>(sx + 8), static_cast<float>(sy + 8), static_cast<float>(slotSize - 16), static_cast<float>(slotSize - 22), C(210, 33, 38, 40));
				DrawTextLine(g_font, slot.label, sx + 7, sy + 24, slotSize - 14, C(255, 245, 245, 240), DT_CENTER | DT_TOP | DT_NOCLIP);
				if (slot.amount > 1)
				{
					char amount[16] = {};
					sprintf(amount, "x%u", static_cast<unsigned>(slot.amount));
					DrawTextLine(g_font, amount, sx + 4, sy + slotSize - 19, slotSize - 8, C(255, 255, 255, 255), DT_RIGHT | DT_TOP | DT_NOCLIP);
				}
			}
		}

		const int detailsX = x + 20 + columns * (slotSize + gap) + 18;
		const int detailsY = y + 104;
		const int detailsW = x + panelW - detailsX - 20;
		DrawRect(device, static_cast<float>(detailsX), static_cast<float>(detailsY), static_cast<float>(detailsW), 344.0f, C(190, 9, 11, 13));
		DrawBorder(device, static_cast<float>(detailsX), static_cast<float>(detailsY), static_cast<float>(detailsW), 344.0f, C(90, 160, 164, 164));

		if (document.hoveredSlot >= 0 && document.hoveredSlot < static_cast<int>(document.slots.size()) && document.slots[document.hoveredSlot].used)
		{
			const InventorySlot& slot = document.slots[document.hoveredSlot];
			DrawTextLine(g_titleFont, slot.label, detailsX + 16, detailsY + 14, detailsW - 32, C(255, 255, 255, 255));
			char meta[64] = {};
			sprintf(meta, "slot %d  |  item %u  |  amount %u", document.hoveredSlot, static_cast<unsigned>(slot.itemId), static_cast<unsigned>(slot.amount));
			DrawTextLine(g_font, meta, detailsX + 16, detailsY + 46, detailsW - 32, C(220, 205, 214, 214));
			DrawRect(device, static_cast<float>(detailsX + 16), static_cast<float>(detailsY + 74), static_cast<float>(detailsW - 32), 1.0f, C(95, 220, 220, 220));
			DrawWrappedText(g_font, slot.description, detailsX + 16, detailsY + 92, detailsW - 32, 130, C(230, 235, 238, 238));
			DrawTextLine(g_font, "LMB use/select  |  RMB secondary", detailsX + 16, detailsY + 304, detailsW - 32, C(210, 255, 255, 255));
		}
		else
		{
			DrawTextLine(g_titleFont, "Inventory", detailsX + 16, detailsY + 14, detailsW - 32, C(255, 255, 255, 255));
			DrawWrappedText(g_font, "Hover a slot to inspect it. Drag a used slot onto another slot to move, merge, or swap. Drop outside the panel to request a world drop.", detailsX + 16, detailsY + 48, detailsW - 32, 150, C(215, 225, 230, 230));
		}

		DrawRect(device, static_cast<float>(x + 14), static_cast<float>(y + panelH - 38), static_cast<float>(panelW - 28), 1.0f, C(80, 230, 230, 230));
		DrawTextLine(g_font, "LMB drag/drop  |  RMB secondary  |  ESC close", x + 18, y + panelH - 27, panelW - 36, C(210, 230, 230, 230));

		if (g_drag.active && g_drag.documentId == document.id)
		{
			const bool outsidePanel = !PointInRect(g_mouse.x, g_mouse.y, document.bounds);
			const int ghostW = 160;
			const int ghostH = 52;
			int gx = g_mouse.x + 18;
			int gy = g_mouse.y + 18;
			if (gx + ghostW > static_cast<int>(viewport.Width))
				gx = g_mouse.x - ghostW - 18;
			if (gy + ghostH > static_cast<int>(viewport.Height))
				gy = g_mouse.y - ghostH - 18;

			DrawRect(device, static_cast<float>(gx), static_cast<float>(gy), static_cast<float>(ghostW), static_cast<float>(ghostH), outsidePanel ? C(230, 58, 24, 12) : C(232, 24, 26, 27));
			DrawBorder(device, static_cast<float>(gx), static_cast<float>(gy), static_cast<float>(ghostW), static_cast<float>(ghostH), outsidePanel ? C(255, 255, 128, 64) : C(230, 255, 255, 255));
			DrawTextLine(g_titleFont, g_drag.item.label, gx + 10, gy + 8, ghostW - 20, C(255, 255, 255, 255));
			DrawTextLine(g_font, outsidePanel ? "drop to world" : "drop into slot", gx + 10, gy + 31, ghostW - 20, outsidePanel ? C(255, 255, 178, 86) : C(220, 225, 232, 232));
		}

		RenderInventoryContextMenu(device, document);
		RenderInventorySplitDialog(device, document);
	}

	void RenderGenericPanel(IDirect3DDevice9* device, UiDocument& document, const D3DVIEWPORT9& viewport)
	{
		const int panelW = 560;
		const int panelH = 360;
		const int x = (static_cast<int>(viewport.Width) - panelW) / 2;
		const int y = (static_cast<int>(viewport.Height) - panelH) / 2;
		RenderPanelBase(device, document, x, y, panelW, panelH);

		DrawWrappedText(g_font, document.body, x + 18, y + 54, panelW - 36, 120, C(230, 232, 236, 236));

		int rowY = y + 180;
		for (std::map<std::string, std::string>::const_iterator it = document.data.begin(); it != document.data.end() && rowY < y + panelH - 52; ++it)
		{
			DrawRect(device, static_cast<float>(x + 18), static_cast<float>(rowY), static_cast<float>(panelW - 36), 28.0f, C(170, 9, 13, 15));
			DrawTextLine(g_font, it->first, x + 28, rowY + 7, 150, C(220, 240, 240, 240));
			DrawTextLine(g_font, it->second, x + 178, rowY + 7, panelW - 210, C(255, 255, 255, 255));
			rowY += 34;
		}

		DrawRect(device, static_cast<float>(x + 14), static_cast<float>(y + panelH - 38), static_cast<float>(panelW - 28), 1.0f, C(80, 230, 230, 230));
		DrawTextLine(g_font, "ESC close", x + 18, y + panelH - 27, panelW - 36, C(210, 230, 230, 230));
	}

	void HandleInventoryClick(UiDocument& document, uint8_t eventType)
	{
		UpdateInventoryHover(document);
		if (document.hoveredSlot < 0)
			return;

		std::string payload;
		if (document.hoveredSlot < static_cast<int>(document.slots.size()) && document.slots[document.hoveredSlot].used)
			payload = document.slots[document.hoveredSlot].label;

		SendUiEvent(document, eventType, static_cast<uint16_t>(document.hoveredSlot), "slot", payload);
	}

	bool HandleInventoryContextMenuClick(UiDocument& document)
	{
		if (!g_contextMenu.active || g_contextMenu.documentId != document.id)
			return false;

		RECT rect = GetContextMenuRect();
		if (!PointInRect(g_mouse.x, g_mouse.y, rect))
		{
			ClearInventoryContextMenu();
			return true;
		}

		const int rowH = 30;
		const int index = (g_mouse.y - (rect.top + 38)) / rowH;
		if (index < 0 || index >= static_cast<int>(g_contextMenu.actions.size()))
			return true;

		if (!IsSlotUsed(document, g_contextMenu.slot))
		{
			ClearInventoryContextMenu();
			return true;
		}

		const std::string actionId = g_contextMenu.actions[index].first;
		const int slot = g_contextMenu.slot;
		if (actionId == "split")
		{
			BeginInventorySplit(document, slot, actionId);
			ClearInventoryContextMenu();
			return true;
		}

		SendUiEvent(
			document,
			OMPPlusProtocol::UiEventInventoryAction,
			static_cast<uint16_t>(slot),
			actionId,
			BuildInventoryActionPayload(document.slots[slot], actionId)
		);
		ClearInventoryContextMenu();
		return true;
	}

	bool HandleInventorySplitClick(UiDocument& document)
	{
		if (!g_split.active || g_split.documentId != document.id)
			return false;

		RECT rect = GetSplitDialogRect(document);
		if (!PointInRect(g_mouse.x, g_mouse.y, rect))
			return true;

		RECT minusButton = { rect.left + 16, rect.top + 114, rect.left + 62, rect.top + 138 };
		RECT plusButton = { rect.left + 70, rect.top + 114, rect.left + 116, rect.top + 138 };
		RECT halfButton = { rect.left + 124, rect.top + 114, rect.left + 194, rect.top + 138 };
		RECT maxButton = { rect.left + 200, rect.top + 114, rect.left + 270, rect.top + 138 };
		RECT splitButton = { rect.left + 16, rect.top + 144, rect.left + 161, rect.top + 168 };
		RECT cancelButton = { rect.left + 179, rect.top + 144, rect.left + 324, rect.top + 168 };

		if (PointInRect(g_mouse.x, g_mouse.y, minusButton))
			g_split.amount = g_split.amount > 1 ? g_split.amount - 1 : 1;
		else if (PointInRect(g_mouse.x, g_mouse.y, plusButton))
			g_split.amount = g_split.amount < g_split.maxAmount ? g_split.amount + 1 : g_split.maxAmount;
		else if (PointInRect(g_mouse.x, g_mouse.y, halfButton))
			g_split.amount = (g_split.maxAmount + 1) / 2 > 1 ? (g_split.maxAmount + 1) / 2 : 1;
		else if (PointInRect(g_mouse.x, g_mouse.y, maxButton))
			g_split.amount = g_split.maxAmount;
		else if (PointInRect(g_mouse.x, g_mouse.y, splitButton))
		{
			if (IsSlotUsed(document, g_split.slot))
			{
				SendUiEvent(
					document,
					OMPPlusProtocol::UiEventInventorySplit,
					static_cast<uint16_t>(g_split.slot),
					g_split.actionId,
					BuildInventorySplitPayload(document.slots[g_split.slot], g_split.amount, g_split.actionId)
				);
			}
			ClearInventorySplit();
		}
		else if (PointInRect(g_mouse.x, g_mouse.y, cancelButton))
		{
			ClearInventorySplit();
		}

		return true;
	}
}

void CRmlUiManager::Initialize(HWND window, IDirect3DDevice9* device)
{
	g_window = window;
	g_device = device;
	g_initialized = window != NULL && device != NULL;

	if (g_initialized && !g_loggedFallback)
	{
		CLog::Write("RmlUi bridge initialized: non-ImGui D3D9 fallback renderer active; real RmlUi core is available as optional C++17 backend.");
		g_loggedFallback = true;
	}
}

void CRmlUiManager::Shutdown()
{
	Clear();

	if (g_font)
	{
		g_font->Release();
		g_font = NULL;
	}
	if (g_titleFont)
	{
		g_titleFont->Release();
		g_titleFont = NULL;
	}

	g_window = NULL;
	g_device = NULL;
	g_initialized = false;
}

void CRmlUiManager::Clear()
{
	const bool hadDocuments = !g_documents.empty();
	g_documents.clear();
	ClearInventoryDrag();
	ClearInventoryContextMenu();
	ClearInventorySplit();
	g_captureActive = false;
	if (hadDocuments)
		CDInput8DeviceProxy::RequestInputReset();
}

void CRmlUiManager::Process()
{
	if (!g_initialized || g_documents.empty())
		return;

	const bool capture = ShouldCaptureMouse();
	if (capture)
	{
		if (!g_captureActive)
		{
			UpdateMouseFromCursor();
			CDInput8DeviceProxy::RequestInputReset();
		}
		ClampMouseToWindow();
	}
	else
	{
		UpdateMouseFromCursor();
	}
	g_captureActive = capture;
}

void CRmlUiManager::Render(IDirect3DDevice9* device)
{
	if (!g_initialized || !device || g_documents.empty())
		return;

	if (!EnsureFonts(device))
		return;

	D3DVIEWPORT9 viewport = {};
	if (FAILED(device->GetViewport(&viewport)) || !viewport.Width || !viewport.Height)
		return;

	IDirect3DStateBlock9* state = NULL;
	if (SUCCEEDED(device->CreateStateBlock(D3DSBT_ALL, &state)) && state)
		state->Capture();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	device->SetRenderState(D3DRS_LIGHTING, FALSE);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device->SetPixelShader(NULL);
	device->SetVertexShader(NULL);

	for (size_t i = 0; i < g_documents.size(); ++i)
	{
		UiDocument& document = g_documents[i];
		if (document.templateId == OMPPlusProtocol::UiTemplateInventory || document.templateId == OMPPlusProtocol::UiTemplateStorage)
			RenderInventory(device, document, viewport);
		else
			RenderGenericPanel(device, document, viewport);
	}

	if (ShouldCaptureMouse())
		DrawCursor(device);

	if (state)
	{
		state->Apply();
		state->Release();
	}
}

void CRmlUiManager::InvalidateDeviceObjects()
{
	if (g_font)
		g_font->OnLostDevice();
	if (g_titleFont)
		g_titleFont->OnLostDevice();
}

void CRmlUiManager::RestoreDeviceObjects()
{
	if (g_font)
		g_font->OnResetDevice();
	if (g_titleFont)
		g_titleFont->OnResetDevice();
}

bool CRmlUiManager::HandleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!g_initialized || g_documents.empty())
		return false;

	if (hWnd && hWnd != g_window)
		g_window = hWnd;

	switch (message)
	{
	case WM_MOUSEMOVE:
		g_mouse.x = GET_X_LPARAM(lParam);
		g_mouse.y = GET_Y_LPARAM(lParam);
		ClampMouseToWindow();
		UpdateInventoryDrag();
		return ShouldCaptureMouse();
	case WM_LBUTTONDOWN:
		ClampMouseToWindow();
		if (UiDocument* document = TopDocument())
		{
			if (IsInventoryTemplate(*document) && HandleInventorySplitClick(*document))
				return true;
			if (IsInventoryTemplate(*document) && HandleInventoryContextMenuClick(*document))
				return true;
			if (IsInventoryTemplate(*document) && BeginInventoryDrag(*document))
				return true;
		}
		return ShouldCaptureMouse();
	case WM_LBUTTONUP:
		ClampMouseToWindow();
		if (UiDocument* document = TopDocument())
		{
			if (IsInventoryTemplate(*document))
			{
				if (!FinishInventoryDrag(*document))
					HandleInventoryClick(*document, OMPPlusProtocol::UiEventClick);
			}
		}
		return ShouldCaptureMouse();
	case WM_RBUTTONUP:
		ClampMouseToWindow();
		if (g_drag.candidate)
		{
			ClearInventoryDrag();
			return true;
		}
		if (g_split.active)
		{
			ClearInventorySplit();
			return true;
		}
		if (g_contextMenu.active)
		{
			ClearInventoryContextMenu();
			return true;
		}
		if (UiDocument* document = TopDocument())
		{
			if (IsInventoryTemplate(*document))
			{
				OpenInventoryContextMenu(*document);
				if (g_contextMenu.active)
					return true;
				HandleInventoryClick(*document, OMPPlusProtocol::UiEventSecondaryClick);
			}
		}
		return ShouldCaptureMouse();
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
		return ShouldCaptureMouse();
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			if (g_split.active)
			{
				ClearInventorySplit();
				return true;
			}
			if (g_contextMenu.active)
			{
				ClearInventoryContextMenu();
				return true;
			}
			UiDocument* top = TopDocument();
			if (top && ShouldCloseOnEscape(*top))
			{
				std::vector<UiDocument>::iterator it = g_documents.end();
				--it;
				CloseDocument(it, true);
				return true;
			}
		}
		return ShouldSuppressKeyboard();
	case WM_KEYUP:
	case WM_SYSKEYUP:
	case WM_CHAR:
		return ShouldSuppressKeyboard();
	default:
		return false;
	}
}

bool CRmlUiManager::ShouldCaptureMouse()
{
	UiDocument* document = TopDocument();
	return document && IsInteractive(*document);
}

bool CRmlUiManager::ShouldNeutralizeKeyboard()
{
	UiDocument* document = TopDocument();
	return document && (document->flags & (OMPPlusProtocol::UiFlagCaptureKeyboard | OMPPlusProtocol::UiFlagModal)) != 0;
}

bool CRmlUiManager::ShouldSuppressKeyboard()
{
	UiDocument* document = TopDocument();
	return document && (document->flags & (OMPPlusProtocol::UiFlagCaptureKeyboard | OMPPlusProtocol::UiFlagModal)) != 0;
}

void CRmlUiManager::FilterKeyboardState(DWORD size, LPVOID state)
{
	if (!state || !size || !ShouldNeutralizeKeyboard())
		return;

	std::memset(state, 0, size);
}

void CRmlUiManager::FilterMouseState(DWORD size, LPVOID state)
{
	if (!state || !size || !ShouldCaptureMouse())
		return;

	if (size >= sizeof(DIMOUSESTATE2))
	{
		DIMOUSESTATE2* mouse = reinterpret_cast<DIMOUSESTATE2*>(state);
		AddMouseDelta(mouse->lX, mouse->lY, mouse->lZ);
		std::memset(mouse, 0, sizeof(DIMOUSESTATE2));
	}
	else if (size >= sizeof(DIMOUSESTATE))
	{
		DIMOUSESTATE* mouse = reinterpret_cast<DIMOUSESTATE*>(state);
		AddMouseDelta(mouse->lX, mouse->lY, mouse->lZ);
		std::memset(mouse, 0, sizeof(DIMOUSESTATE));
	}
}

bool CRmlUiManager::ShouldConsumeDirectInputEvent(DWORD, DWORD)
{
	return ShouldNeutralizeKeyboard();
}

void CRmlUiManager::AddMouseDelta(LONG x, LONG y, LONG)
{
	if (!ShouldCaptureMouse())
		return;

	g_mouse.x += x;
	g_mouse.y += y;
	ClampMouseToWindow();
	UpdateInventoryDrag();
}

void CRmlUiManager::HandleOpen(RakNet::BitStream& stream)
{
	UiDocument document;
	if (!stream.Read(document.templateId)
		|| !stream.Read(document.flags)
		|| !stream.Read(document.capacity)
		|| !ReadBoundString(stream, document.id, MaxDocumentIdLength)
		|| !ReadBoundString(stream, document.title, MaxTitleLength)
		|| !ReadBoundString(stream, document.body, MaxBodyLength))
	{
		return;
	}

	if (document.id.empty())
		return;

	document.capacity = document.capacity < MaxInventorySlots ? document.capacity : MaxInventorySlots;
	if ((document.templateId == OMPPlusProtocol::UiTemplateInventory || document.templateId == OMPPlusProtocol::UiTemplateStorage) && document.capacity == 0)
		document.capacity = 30;

	if (document.capacity)
		document.slots.resize(document.capacity);

	std::vector<UiDocument>::iterator existing = FindDocument(document.id);
	if (existing != g_documents.end())
	{
		if (g_drag.documentId == document.id)
			ClearInventoryDrag();
		g_documents.erase(existing);
	}

	g_documents.push_back(document);
	UpdateMouseFromCursor();
	g_captureActive = false;
	CDInput8DeviceProxy::RequestInputReset();
}

void CRmlUiManager::HandleClose(RakNet::BitStream& stream)
{
	std::string id;
	if (!ReadBoundString(stream, id, MaxDocumentIdLength) || id.empty())
		return;

	CloseDocument(FindDocument(id), false);
}

void CRmlUiManager::HandleCloseAll()
{
	Clear();
}

void CRmlUiManager::HandleSetData(RakNet::BitStream& stream)
{
	std::string id;
	std::string key;
	std::string value;
	if (!ReadBoundString(stream, id, MaxDocumentIdLength)
		|| !ReadBoundString(stream, key, MaxKeyLength)
		|| !ReadBoundString(stream, value, MaxValueLength)
		|| id.empty()
		|| key.empty())
	{
		return;
	}

	std::vector<UiDocument>::iterator it = FindDocument(id);
	if (it == g_documents.end())
		return;

	if (key == "title")
		it->title = value;
	else if (key == "body")
		it->body = value;
	else
		it->data[key] = value;
}

void CRmlUiManager::HandleInventoryClear(RakNet::BitStream& stream)
{
	std::string id;
	if (!ReadBoundString(stream, id, MaxDocumentIdLength) || id.empty())
		return;

	std::vector<UiDocument>::iterator it = FindDocument(id);
	if (it == g_documents.end())
		return;

	for (size_t i = 0; i < it->slots.size(); ++i)
		it->slots[i] = InventorySlot();
}

void CRmlUiManager::HandleInventorySetSlot(RakNet::BitStream& stream)
{
	std::string id;
	uint16_t slot = 0;
	InventorySlot value;
	if (!ReadBoundString(stream, id, MaxDocumentIdLength)
		|| !stream.Read(slot)
		|| !stream.Read(value.itemId)
		|| !stream.Read(value.amount)
		|| !ReadBoundString(stream, value.label, MaxSlotLabelLength)
		|| !ReadBoundString(stream, value.description, MaxSlotDescriptionLength)
		|| !ReadBoundString(stream, value.icon, MaxIconLength))
	{
		return;
	}

	std::vector<UiDocument>::iterator it = FindDocument(id);
	if (it == g_documents.end() || slot >= MaxInventorySlots)
		return;

	if (it->slots.size() <= slot)
		it->slots.resize(slot + 1);
	if (it->capacity <= slot)
		it->capacity = slot + 1;

	value.used = value.itemId != 0 || !value.label.empty();
	if (it->slots[slot].used && !it->slots[slot].actions.empty())
		value.actions = it->slots[slot].actions;
	it->slots[slot] = value;
}

void CRmlUiManager::HandleInventorySetSlotActions(RakNet::BitStream& stream)
{
	std::string id;
	uint16_t slot = 0;
	std::string actions;
	if (!ReadBoundString(stream, id, MaxDocumentIdLength)
		|| !stream.Read(slot)
		|| !ReadBoundString(stream, actions, MaxActionsLength))
	{
		return;
	}

	std::vector<UiDocument>::iterator it = FindDocument(id);
	if (it == g_documents.end() || slot >= MaxInventorySlots)
		return;

	if (it->slots.size() <= slot)
		it->slots.resize(slot + 1);
	if (it->capacity <= slot)
		it->capacity = slot + 1;

	it->slots[slot].actions = ParseInventoryActions(actions);
}
