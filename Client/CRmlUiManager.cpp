#include <SAMP+/client/CRmlUiManager.h>

#include <SAMP+/OMPPlusProtocol.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/Network.h>

#include <DirectX/d3dx9.h>
#include <windowsx.h>

#include <algorithm>
#include <map>
#include <stdio.h>
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

	const DWORD UiFvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
	const size_t MaxDocumentIdLength = 31;
	const size_t MaxTitleLength = 64;
	const size_t MaxBodyLength = 255;
	const size_t MaxKeyLength = 31;
	const size_t MaxValueLength = 255;
	const size_t MaxSlotLabelLength = 48;
	const size_t MaxSlotDescriptionLength = 96;
	const size_t MaxIconLength = 48;
	const uint16_t MaxInventorySlots = 120;

	static HWND g_window = NULL;
	static IDirect3DDevice9* g_device = NULL;
	static ID3DXFont* g_font = NULL;
	static ID3DXFont* g_titleFont = NULL;
	static std::vector<UiDocument> g_documents;
	static POINT g_mouse = { 0, 0 };
	static bool g_initialized = false;
	static bool g_loggedFallback = false;

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
			g_mouse = point;
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

	void CloseDocument(std::vector<UiDocument>::iterator it, bool notify)
	{
		if (it == g_documents.end())
			return;

		if (notify)
			SendUiEvent(*it, OMPPlusProtocol::UiEventClose, 0, "document", "");

		g_documents.erase(it);
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

	void RenderInventory(IDirect3DDevice9* device, UiDocument& document, const D3DVIEWPORT9& viewport)
	{
		const int panelW = 780;
		const int panelH = 548;
		const int x = (static_cast<int>(viewport.Width) - panelW) / 2;
		const int y = (static_cast<int>(viewport.Height) - panelH) / 2;
		RenderPanelBase(device, document, x, y, panelW, panelH);

		DrawWrappedText(g_font, document.body, x + 18, y + 48, panelW - 36, 42, C(220, 230, 234, 232));

		const int slotSize = 62;
		const int gap = 8;
		const int columns = 6;
		const int startX = x + 20;
		const int startY = y + 104;
		const int maxRows = 5;
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

			DrawRect(device, static_cast<float>(sx), static_cast<float>(sy), static_cast<float>(slotSize), static_cast<float>(slotSize), hovered ? C(238, 68, 72, 76) : C(205, 11, 15, 17));
			DrawBorder(device, static_cast<float>(sx), static_cast<float>(sy), static_cast<float>(slotSize), static_cast<float>(slotSize), hovered ? C(230, 245, 245, 245) : C(96, 95, 104, 104));

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
			DrawWrappedText(g_font, "Hover a slot to inspect it. Click events are sent back to Pawn with the document id, slot index, and event type.", detailsX + 16, detailsY + 48, detailsW - 32, 130, C(215, 225, 230, 230));
		}

		DrawRect(device, static_cast<float>(x + 14), static_cast<float>(y + panelH - 38), static_cast<float>(panelW - 28), 1.0f, C(80, 230, 230, 230));
		DrawTextLine(g_font, "ESC close", x + 18, y + panelH - 27, panelW - 36, C(210, 230, 230, 230));
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
		if (document.hoveredSlot < 0)
			return;

		std::string payload;
		if (document.hoveredSlot < static_cast<int>(document.slots.size()) && document.slots[document.hoveredSlot].used)
			payload = document.slots[document.hoveredSlot].label;

		SendUiEvent(document, eventType, static_cast<uint16_t>(document.hoveredSlot), "slot", payload);
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
	g_documents.clear();
}

void CRmlUiManager::Process()
{
	if (!g_initialized || g_documents.empty())
		return;

	UpdateMouseFromCursor();
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
		return ShouldCaptureMouse();
	case WM_LBUTTONUP:
		UpdateMouseFromCursor();
		if (UiDocument* document = TopDocument())
		{
			if (document->templateId == OMPPlusProtocol::UiTemplateInventory || document->templateId == OMPPlusProtocol::UiTemplateStorage)
				HandleInventoryClick(*document, OMPPlusProtocol::UiEventClick);
		}
		return ShouldCaptureMouse();
	case WM_RBUTTONUP:
		UpdateMouseFromCursor();
		if (UiDocument* document = TopDocument())
		{
			if (document->templateId == OMPPlusProtocol::UiTemplateInventory || document->templateId == OMPPlusProtocol::UiTemplateStorage)
				HandleInventoryClick(*document, OMPPlusProtocol::UiEventSecondaryClick);
		}
		return ShouldCaptureMouse();
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
		return ShouldCaptureMouse();
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam == VK_ESCAPE)
		{
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

bool CRmlUiManager::ShouldSuppressKeyboard()
{
	UiDocument* document = TopDocument();
	return document && (document->flags & (OMPPlusProtocol::UiFlagCaptureKeyboard | OMPPlusProtocol::UiFlagModal)) != 0;
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
		g_documents.erase(existing);

	g_documents.push_back(document);
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
	it->slots[slot] = value;
}
