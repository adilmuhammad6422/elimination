// dear imgui, v1.52 WIP
// (drawing and font code)

// Contains implementation for
// - ImDrawList
// - ImDrawData
// - ImFontAtlas
// - ImFont
// - Default font data

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_PLACEMENT_NEW
#include "imgui_internal.h"

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#ifdef _WIN32
#include <malloc.h>     // alloca
#elif defined(__GLIBC__) || defined(__sun)
#include <alloca.h>     // alloca
#else
#include <stdlib.h>     // alloca
#endif
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#define snprintf _snprintf
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference it.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"      // warning : macro name is a reserved identifier                //
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'xxxx' to type 'xxxx' casts away qualifiers
#endif

//-------------------------------------------------------------------------
// STB libraries implementation
//-------------------------------------------------------------------------

//#define IMGUI_STB_NAMESPACE     ImGuiStb
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#endif

#define STBRP_ASSERT(x)    IM_ASSERT(x)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#include "stb_rect_pack.h"

#define STBTT_malloc(x,u)  ((void)(u), ImGui::MemAlloc(x))
#define STBTT_free(x,u)    ((void)(u), ImGui::MemFree(x))
#define STBTT_assert(x)    IM_ASSERT(x)
#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF extern
#endif
#include "stb_truetype.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImGuiStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// ImDrawList
//-----------------------------------------------------------------------------

static const ImVec4 GNullClipRect(-8192.0f, -8192.0f, +8192.0f, +8192.0f); // Large values that are easy to encode in a few bits+shift

void ImDrawList::Clear()
{
	CmdBuffer.resize(0);
	IdxBuffer.resize(0);
	VtxBuffer.resize(0);
	_VtxCurrentIdx = 0;
	_VtxWritePtr = NULL;
	_IdxWritePtr = NULL;
	_ClipRectStack.resize(0);
	_TextureIdStack.resize(0);
	_Path.resize(0);
	_ChannelsCurrent = 0;
	_ChannelsCount = 1;
	// NB: Do not clear channels so our allocations are re-used after the first frame.
}

void ImDrawList::ClearFreeMemory()
{
	CmdBuffer.clear();
	IdxBuffer.clear();
	VtxBuffer.clear();
	_VtxCurrentIdx = 0;
	_VtxWritePtr = NULL;
	_IdxWritePtr = NULL;
	_ClipRectStack.clear();
	_TextureIdStack.clear();
	_Path.clear();
	_ChannelsCurrent = 0;
	_ChannelsCount = 1;
	for (int i = 0; i < _Channels.Size; i++)
	{
		if (i == 0) memset(&_Channels[0], 0, sizeof(_Channels[0]));  // channel 0 is a copy of CmdBuffer/IdxBuffer, don't destruct again
		_Channels[i].CmdBuffer.clear();
		_Channels[i].IdxBuffer.clear();
	}
	_Channels.clear();
}

// Use macros because C++ is a terrible language, we want guaranteed inline, no code in header, and no overhead in Debug mode
#define GetCurrentClipRect()    (_ClipRectStack.Size ? _ClipRectStack.Data[_ClipRectStack.Size-1]  : GNullClipRect)
#define GetCurrentTextureId()   (_TextureIdStack.Size ? _TextureIdStack.Data[_TextureIdStack.Size-1] : NULL)

void ImDrawList::AddDrawCmd()
{
	ImDrawCmd draw_cmd;
	draw_cmd.ClipRect = GetCurrentClipRect();
	draw_cmd.TextureId = GetCurrentTextureId();

	IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
	CmdBuffer.push_back(draw_cmd);
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* callback_data)
{
	ImDrawCmd* current_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
	if (!current_cmd || current_cmd->ElemCount != 0 || current_cmd->UserCallback != NULL)
	{
		AddDrawCmd();
		current_cmd = &CmdBuffer.back();
	}
	current_cmd->UserCallback = callback;
	current_cmd->UserCallbackData = callback_data;

	AddDrawCmd(); // Force a new command after us (see comment below)
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::UpdateClipRect()
{
	// If current command is used with different settings we need to add a new command
	const ImVec4 curr_clip_rect = GetCurrentClipRect();
	ImDrawCmd* curr_cmd = CmdBuffer.Size > 0 ? &CmdBuffer.Data[CmdBuffer.Size - 1] : NULL;
	if (!curr_cmd || (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) != 0) || curr_cmd->UserCallback != NULL)
	{
		AddDrawCmd();
		return;
	}

	// Try to merge with previous command if it matches, else use current command
	ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
	if (curr_cmd->ElemCount == 0 && prev_cmd && memcmp(&prev_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) == 0 && prev_cmd->TextureId == GetCurrentTextureId() && prev_cmd->UserCallback == NULL)
		CmdBuffer.pop_back();
	else
		curr_cmd->ClipRect = curr_clip_rect;
}

void ImDrawList::UpdateTextureID()
{
	// If current command is used with different settings we need to add a new command
	const ImTextureID curr_texture_id = GetCurrentTextureId();
	ImDrawCmd* curr_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
	if (!curr_cmd || (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != curr_texture_id) || curr_cmd->UserCallback != NULL)
	{
		AddDrawCmd();
		return;
	}

	// Try to merge with previous command if it matches, else use current command
	ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
	if (prev_cmd && prev_cmd->TextureId == curr_texture_id && memcmp(&prev_cmd->ClipRect, &GetCurrentClipRect(), sizeof(ImVec4)) == 0 && prev_cmd->UserCallback == NULL)
		CmdBuffer.pop_back();
	else
		curr_cmd->TextureId = curr_texture_id;
}

#undef GetCurrentClipRect
#undef GetCurrentTextureId

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(ImVec2 cr_min, ImVec2 cr_max, bool intersect_with_current_clip_rect)
{
	ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
	if (intersect_with_current_clip_rect && _ClipRectStack.Size)
	{
		ImVec4 current = _ClipRectStack.Data[_ClipRectStack.Size - 1];
		if (cr.x < current.x) cr.x = current.x;
		if (cr.y < current.y) cr.y = current.y;
		if (cr.z > current.z) cr.z = current.z;
		if (cr.w > current.w) cr.w = current.w;
	}
	cr.z = ImMax(cr.x, cr.z);
	cr.w = ImMax(cr.y, cr.w);

	_ClipRectStack.push_back(cr);
	UpdateClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
	PushClipRect(ImVec2(GNullClipRect.x, GNullClipRect.y), ImVec2(GNullClipRect.z, GNullClipRect.w));
	//PushClipRect(GetVisibleRect());   // FIXME-OPT: This would be more correct but we're not supposed to access ImGuiContext from here?
}

void ImDrawList::PopClipRect()
{
	IM_ASSERT(_ClipRectStack.Size > 0);
	_ClipRectStack.pop_back();
	UpdateClipRect();
}

void ImDrawList::PushTextureID(const ImTextureID& texture_id)
{
	_TextureIdStack.push_back(texture_id);
	UpdateTextureID();
}

void ImDrawList::PopTextureID()
{
	IM_ASSERT(_TextureIdStack.Size > 0);
	_TextureIdStack.pop_back();
	UpdateTextureID();
}

void ImDrawList::ChannelsSplit(int channels_count)
{
	IM_ASSERT(_ChannelsCurrent == 0 && _ChannelsCount == 1);
	int old_channels_count = _Channels.Size;
	if (old_channels_count < channels_count)
		_Channels.resize(channels_count);
	_ChannelsCount = channels_count;

	// _Channels[] (24 bytes each) hold storage that we'll swap with this->_CmdBuffer/_IdxBuffer
	// The content of _Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
	// When we switch to the next channel, we'll copy _CmdBuffer/_IdxBuffer into _Channels[0] and then _Channels[1] into _CmdBuffer/_IdxBuffer
	memset(&_Channels[0], 0, sizeof(ImDrawChannel));
	for (int i = 1; i < channels_count; i++)
	{
		if (i >= old_channels_count)
		{
			IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
		}
		else
		{
			_Channels[i].CmdBuffer.resize(0);
			_Channels[i].IdxBuffer.resize(0);
		}
		if (_Channels[i].CmdBuffer.Size == 0)
		{
			ImDrawCmd draw_cmd;
			draw_cmd.ClipRect = _ClipRectStack.back();
			draw_cmd.TextureId = _TextureIdStack.back();
			_Channels[i].CmdBuffer.push_back(draw_cmd);
		}
	}
}

void ImDrawList::ChannelsMerge()
{
	// Note that we never use or rely on channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
	if (_ChannelsCount <= 1)
		return;

	ChannelsSetCurrent(0);
	if (CmdBuffer.Size && CmdBuffer.back().ElemCount == 0)
		CmdBuffer.pop_back();

	int new_cmd_buffer_count = 0, new_idx_buffer_count = 0;
	for (int i = 1; i < _ChannelsCount; i++)
	{
		ImDrawChannel& ch = _Channels[i];
		if (ch.CmdBuffer.Size && ch.CmdBuffer.back().ElemCount == 0)
			ch.CmdBuffer.pop_back();
		new_cmd_buffer_count += ch.CmdBuffer.Size;
		new_idx_buffer_count += ch.IdxBuffer.Size;
	}
	CmdBuffer.resize(CmdBuffer.Size + new_cmd_buffer_count);
	IdxBuffer.resize(IdxBuffer.Size + new_idx_buffer_count);

	ImDrawCmd* cmd_write = CmdBuffer.Data + CmdBuffer.Size - new_cmd_buffer_count;
	_IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size - new_idx_buffer_count;
	for (int i = 1; i < _ChannelsCount; i++)
	{
		ImDrawChannel& ch = _Channels[i];
		if (int sz = ch.CmdBuffer.Size) { memcpy(cmd_write, ch.CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
		if (int sz = ch.IdxBuffer.Size) { memcpy(_IdxWritePtr, ch.IdxBuffer.Data, sz * sizeof(ImDrawIdx)); _IdxWritePtr += sz; }
	}
	AddDrawCmd();
	_ChannelsCount = 1;
}

void ImDrawList::ChannelsSetCurrent(int idx)
{
	IM_ASSERT(idx < _ChannelsCount);
	if (_ChannelsCurrent == idx) return;
	memcpy(&_Channels.Data[_ChannelsCurrent].CmdBuffer, &CmdBuffer, sizeof(CmdBuffer)); // copy 12 bytes, four times
	memcpy(&_Channels.Data[_ChannelsCurrent].IdxBuffer, &IdxBuffer, sizeof(IdxBuffer));
	_ChannelsCurrent = idx;
	memcpy(&CmdBuffer, &_Channels.Data[_ChannelsCurrent].CmdBuffer, sizeof(CmdBuffer));
	memcpy(&IdxBuffer, &_Channels.Data[_ChannelsCurrent].IdxBuffer, sizeof(IdxBuffer));
	_IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size;
}

// NB: this can be called with negative count for removing primitives (as long as the result does not underflow)
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
	ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size - 1];
	draw_cmd.ElemCount += idx_count;

	int vtx_buffer_old_size = VtxBuffer.Size;
	VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
	_VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

	int idx_buffer_old_size = IdxBuffer.Size;
	IdxBuffer.resize(idx_buffer_old_size + idx_count);
	_IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
	ImVec2 b(c.x, a.y), d(a.x, c.y), uv(GImGui->FontTexUvWhitePixel);
	ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
	_IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
	_IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
	_VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
	_VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
	_VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
	_VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
	_VtxWritePtr += 4;
	_VtxCurrentIdx += 4;
	_IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
	ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
	ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
	_IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
	_IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
	_VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
	_VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
	_VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
	_VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
	_VtxWritePtr += 4;
	_VtxCurrentIdx += 4;
	_IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
	ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
	_IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
	_IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
	_VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
	_VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
	_VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
	_VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
	_VtxWritePtr += 4;
	_VtxCurrentIdx += 4;
	_IdxWritePtr += 6;
}

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, bool closed, float thickness, bool anti_aliased)
{
	if (points_count < 2)
		return;

	const ImVec2 uv = GImGui->FontTexUvWhitePixel;
	anti_aliased &= GImGui->Style.AntiAliasedLines;
	//if (ImGui::GetIO().KeyCtrl) anti_aliased = false; // Debug

	int count = points_count;
	if (!closed)
		count = points_count - 1;

	const bool thick_line = thickness > 1.0f;
	if (anti_aliased)
	{
		// Anti-aliased stroke
		const float AA_SIZE = 1.0f;
		const ImU32 col_trans = col & ~IM_COL32_A_MASK;

		const int idx_count = thick_line ? count * 18 : count * 12;
		const int vtx_count = thick_line ? points_count * 4 : points_count * 3;
		PrimReserve(idx_count, vtx_count);

		// Temporary buffer
		ImVec2* temp_normals = (ImVec2*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(ImVec2));
		ImVec2* temp_points = temp_normals + points_count;

		for (int i1 = 0; i1 < count; i1++)
		{
			const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
			ImVec2 diff = points[i2] - points[i1];
			diff *= ImInvLength(diff, 1.0f);
			temp_normals[i1].x = diff.y;
			temp_normals[i1].y = -diff.x;
		}
		if (!closed)
			temp_normals[points_count - 1] = temp_normals[points_count - 2];

		if (!thick_line)
		{
			if (!closed)
			{
				temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
				temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
				temp_points[(points_count - 1) * 2 + 0] = points[points_count - 1] + temp_normals[points_count - 1] * AA_SIZE;
				temp_points[(points_count - 1) * 2 + 1] = points[points_count - 1] - temp_normals[points_count - 1] * AA_SIZE;
			}

			// FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
			unsigned int idx1 = _VtxCurrentIdx;
			for (int i1 = 0; i1 < count; i1++)
			{
				const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
				unsigned int idx2 = (i1 + 1) == points_count ? _VtxCurrentIdx : idx1 + 3;

				// Average normals
				ImVec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
				float dmr2 = dm.x*dm.x + dm.y*dm.y;
				if (dmr2 > 0.000001f)
				{
					float scale = 1.0f / dmr2;
					if (scale > 100.0f) scale = 100.0f;
					dm *= scale;
				}
				dm *= AA_SIZE;
				temp_points[i2 * 2 + 0] = points[i2] + dm;
				temp_points[i2 * 2 + 1] = points[i2] - dm;

				// Add indexes
				_IdxWritePtr[0] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 2);
				_IdxWritePtr[3] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 0);
				_IdxWritePtr[6] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8] = (ImDrawIdx)(idx1 + 0);
				_IdxWritePtr[9] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1);
				_IdxWritePtr += 12;

				idx1 = idx2;
			}

			// Add vertexes
			for (int i = 0; i < points_count; i++)
			{
				_VtxWritePtr[0].pos = points[i];          _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
				_VtxWritePtr[1].pos = temp_points[i * 2 + 0]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;
				_VtxWritePtr[2].pos = temp_points[i * 2 + 1]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col_trans;
				_VtxWritePtr += 3;
			}
		}
		else
		{
			const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
			if (!closed)
			{
				temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
				temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
				temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
				temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
				temp_points[(points_count - 1) * 4 + 0] = points[points_count - 1] + temp_normals[points_count - 1] * (half_inner_thickness + AA_SIZE);
				temp_points[(points_count - 1) * 4 + 1] = points[points_count - 1] + temp_normals[points_count - 1] * (half_inner_thickness);
				temp_points[(points_count - 1) * 4 + 2] = points[points_count - 1] - temp_normals[points_count - 1] * (half_inner_thickness);
				temp_points[(points_count - 1) * 4 + 3] = points[points_count - 1] - temp_normals[points_count - 1] * (half_inner_thickness + AA_SIZE);
			}

			// FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
			unsigned int idx1 = _VtxCurrentIdx;
			for (int i1 = 0; i1 < count; i1++)
			{
				const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
				unsigned int idx2 = (i1 + 1) == points_count ? _VtxCurrentIdx : idx1 + 4;

				// Average normals
				ImVec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
				float dmr2 = dm.x*dm.x + dm.y*dm.y;
				if (dmr2 > 0.000001f)
				{
					float scale = 1.0f / dmr2;
					if (scale > 100.0f) scale = 100.0f;
					dm *= scale;
				}
				ImVec2 dm_out = dm * (half_inner_thickness + AA_SIZE);
				ImVec2 dm_in = dm * half_inner_thickness;
				temp_points[i2 * 4 + 0] = points[i2] + dm_out;
				temp_points[i2 * 4 + 1] = points[i2] + dm_in;
				temp_points[i2 * 4 + 2] = points[i2] - dm_in;
				temp_points[i2 * 4 + 3] = points[i2] - dm_out;

				// Add indexes
				_IdxWritePtr[0] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 2);
				_IdxWritePtr[3] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 1);
				_IdxWritePtr[6] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8] = (ImDrawIdx)(idx1 + 0);
				_IdxWritePtr[9] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1);
				_IdxWritePtr[12] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[13] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[14] = (ImDrawIdx)(idx1 + 3);
				_IdxWritePtr[15] = (ImDrawIdx)(idx1 + 3); _IdxWritePtr[16] = (ImDrawIdx)(idx2 + 3); _IdxWritePtr[17] = (ImDrawIdx)(idx2 + 2);
				_IdxWritePtr += 18;

				idx1 = idx2;
			}

			// Add vertexes
			for (int i = 0; i < points_count; i++)
			{
				_VtxWritePtr[0].pos = temp_points[i * 4 + 0]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col_trans;
				_VtxWritePtr[1].pos = temp_points[i * 4 + 1]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
				_VtxWritePtr[2].pos = temp_points[i * 4 + 2]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
				_VtxWritePtr[3].pos = temp_points[i * 4 + 3]; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col_trans;
				_VtxWritePtr += 4;
			}
		}
		_VtxCurrentIdx += (ImDrawIdx)vtx_count;
	}
	else
	{
		// Non Anti-aliased Stroke
		const int idx_count = count * 6;
		const int vtx_count = count * 4;      // FIXME-OPT: Not sharing edges
		PrimReserve(idx_count, vtx_count);

		for (int i1 = 0; i1 < count; i1++)
		{
			const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
			const ImVec2& p1 = points[i1];
			const ImVec2& p2 = points[i2];
			ImVec2 diff = p2 - p1;
			diff *= ImInvLength(diff, 1.0f);

			const float dx = diff.x * (thickness * 0.5f);
			const float dy = diff.y * (thickness * 0.5f);
			_VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
			_VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
			_VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
			_VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
			_VtxWritePtr += 4;

			_IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + 2);
			_IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx + 2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx + 3);
			_IdxWritePtr += 6;
			_VtxCurrentIdx += 4;
		}
	}
}

void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col, bool anti_aliased)
{
	const ImVec2 uv = GImGui->FontTexUvWhitePixel;
	anti_aliased &= GImGui->Style.AntiAliasedShapes;
	//if (ImGui::GetIO().KeyCtrl) anti_aliased = false; // Debug

	if (anti_aliased)
	{
		// Anti-aliased Fill
		const float AA_SIZE = 1.0f;
		const ImU32 col_trans = col & ~IM_COL32_A_MASK;
		const int idx_count = (points_count - 2) * 3 + points_count * 6;
		const int vtx_count = (points_count * 2);
		PrimReserve(idx_count, vtx_count);

		// Add indexes for fill
		unsigned int vtx_inner_idx = _VtxCurrentIdx;
		unsigned int vtx_outer_idx = _VtxCurrentIdx + 1;
		for (int i = 2; i < points_count; i++)
		{
			_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + ((i - 1) << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx + (i << 1));
			_IdxWritePtr += 3;
		}

		// Compute normals
		ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2));
		for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
		{
			const ImVec2& p0 = points[i0];
			const ImVec2& p1 = points[i1];
			ImVec2 diff = p1 - p0;
			diff *= ImInvLength(diff, 1.0f);
			temp_normals[i0].x = diff.y;
			temp_normals[i0].y = -diff.x;
		}

		for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
		{
			// Average normals
			const ImVec2& n0 = temp_normals[i0];
			const ImVec2& n1 = temp_normals[i1];
			ImVec2 dm = (n0 + n1) * 0.5f;
			float dmr2 = dm.x*dm.x + dm.y*dm.y;
			if (dmr2 > 0.000001f)
			{
				float scale = 1.0f / dmr2;
				if (scale > 100.0f) scale = 100.0f;
				dm *= scale;
			}
			dm *= AA_SIZE * 0.5f;

			// Add vertices
			_VtxWritePtr[0].pos = (points[i1] - dm); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
			_VtxWritePtr[1].pos = (points[i1] + dm); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
			_VtxWritePtr += 2;

			// Add indexes for fringes
			_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + (i0 << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1));
			_IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx + (i1 << 1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1));
			_IdxWritePtr += 6;
		}
		_VtxCurrentIdx += (ImDrawIdx)vtx_count;
	}
	else
	{
		// Non Anti-aliased Fill
		const int idx_count = (points_count - 2) * 3;
		const int vtx_count = points_count;
		PrimReserve(idx_count, vtx_count);
		for (int i = 0; i < vtx_count; i++)
		{
			_VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
			_VtxWritePtr++;
		}
		for (int i = 2; i < points_count; i++)
		{
			_IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + i - 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + i);
			_IdxWritePtr += 3;
		}
		_VtxCurrentIdx += (ImDrawIdx)vtx_count;
	}
}

void ImDrawList::PathArcToFast(const ImVec2& centre, float radius, int a_min_of_12, int a_max_of_12)
{
	static ImVec2 circle_vtx[12];
	static bool circle_vtx_builds = false;
	const int circle_vtx_count = IM_ARRAYSIZE(circle_vtx);
	if (!circle_vtx_builds)
	{
		for (int i = 0; i < circle_vtx_count; i++)
		{
			const float a = ((float)i / (float)circle_vtx_count) * 2 * IM_PI;
			circle_vtx[i].x = cosf(a);
			circle_vtx[i].y = sinf(a);
		}
		circle_vtx_builds = true;
	}

	if (radius == 0.0f || a_min_of_12 > a_max_of_12)
	{
		_Path.push_back(centre);
		return;
	}
	_Path.reserve(_Path.Size + (a_max_of_12 - a_min_of_12 + 1));
	for (int a = a_min_of_12; a <= a_max_of_12; a++)
	{
		const ImVec2& c = circle_vtx[a % circle_vtx_count];
		_Path.push_back(ImVec2(centre.x + c.x * radius, centre.y + c.y * radius));
	}
}

void ImDrawList::PathArcTo(const ImVec2& centre, float radius, float a_min, float a_max, int num_segments)
{
	if (radius == 0.0f)
	{
		_Path.push_back(centre);
		return;
	}
	_Path.reserve(_Path.Size + (num_segments + 1));
	for (int i = 0; i <= num_segments; i++)
	{
		const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
		_Path.push_back(ImVec2(centre.x + cosf(a) * radius, centre.y + sinf(a) * radius));
	}
}

static void PathBezierToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
	float dx = x4 - x1;
	float dy = y4 - y1;
	float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
	float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
	d2 = (d2 >= 0) ? d2 : -d2;
	d3 = (d3 >= 0) ? d3 : -d3;
	if ((d2 + d3) * (d2 + d3) < tess_tol * (dx*dx + dy*dy))
	{
		path->push_back(ImVec2(x4, y4));
	}
	else if (level < 10)
	{
		float x12 = (x1 + x2)*0.5f, y12 = (y1 + y2)*0.5f;
		float x23 = (x2 + x3)*0.5f, y23 = (y2 + y3)*0.5f;
		float x34 = (x3 + x4)*0.5f, y34 = (y3 + y4)*0.5f;
		float x123 = (x12 + x23)*0.5f, y123 = (y12 + y23)*0.5f;
		float x234 = (x23 + x34)*0.5f, y234 = (y23 + y34)*0.5f;
		float x1234 = (x123 + x234)*0.5f, y1234 = (y123 + y234)*0.5f;

		PathBezierToCasteljau(path, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
		PathBezierToCasteljau(path, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
	}
}

void ImDrawList::PathBezierCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
	ImVec2 p1 = _Path.back();
	if (num_segments == 0)
	{
		// Auto-tessellated
		PathBezierToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, GImGui->Style.CurveTessellationTol, 0);
	}
	else
	{
		float t_step = 1.0f / (float)num_segments;
		for (int i_step = 1; i_step <= num_segments; i_step++)
		{
			float t = t_step * i_step;
			float u = 1.0f - t;
			float w1 = u*u*u;
			float w2 = 3 * u*u*t;
			float w3 = 3 * u*t*t;
			float w4 = t*t*t;
			_Path.push_back(ImVec2(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
		}
	}
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, int rounding_corners)
{
	const int corners_top = ImGuiCorner_TopLeft | ImGuiCorner_TopRight;
	const int corners_bottom = ImGuiCorner_BotLeft | ImGuiCorner_BotRight;
	const int corners_left = ImGuiCorner_TopLeft | ImGuiCorner_BotLeft;
	const int corners_right = ImGuiCorner_TopRight | ImGuiCorner_BotRight;

	rounding = ImMin(rounding, fabsf(b.x - a.x) * (((rounding_corners & corners_top) == corners_top) || ((rounding_corners & corners_bottom) == corners_bottom) ? 0.5f : 1.0f) - 1.0f);
	rounding = ImMin(rounding, fabsf(b.y - a.y) * (((rounding_corners & corners_left) == corners_left) || ((rounding_corners & corners_right) == corners_right) ? 0.5f : 1.0f) - 1.0f);

	if (rounding <= 0.0f || rounding_corners == 0)
	{
		PathLineTo(a);
		PathLineTo(ImVec2(b.x, a.y));
		PathLineTo(b);
		PathLineTo(ImVec2(a.x, b.y));
	}
	else
	{
		const float rounding_tl = (rounding_corners & ImGuiCorner_TopLeft) ? rounding : 0.0f;
		const float rounding_tr = (rounding_corners & ImGuiCorner_TopRight) ? rounding : 0.0f;
		const float rounding_br = (rounding_corners & ImGuiCorner_BotRight) ? rounding : 0.0f;
		const float rounding_bl = (rounding_corners & ImGuiCorner_BotLeft) ? rounding : 0.0f;
		PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
		PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
		PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
		PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
	}
}

void ImDrawList::AddLine(const ImVec2& a, const ImVec2& b, ImU32 col, float thickness)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;
	PathLineTo(a + ImVec2(0.5f, 0.5f));
	PathLineTo(b + ImVec2(0.5f, 0.5f));
	PathStroke(col, false, thickness);
}

// a: upper-left, b: lower-right. we don't render 1 px sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags, float thickness)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;
	PathRect(a + ImVec2(0.5f, 0.5f), b - ImVec2(0.5f, 0.5f), rounding, rounding_corners_flags);
	PathStroke(col, true, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;
	if (rounding > 0.0f)
	{
		PathRect(a, b, rounding, rounding_corners_flags);
		PathFillConvex(col);
	}
	else
	{
		PrimReserve(6, 4);
		PrimRect(a, b, col);
	}
}

void ImDrawList::AddRectFilledMultiColor(const ImVec2& a, const ImVec2& c, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
	if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
		return;

	const ImVec2 uv = GImGui->FontTexUvWhitePixel;
	PrimReserve(6, 4);
	PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2));
	PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 3));
	PrimWriteVtx(a, uv, col_upr_left);
	PrimWriteVtx(ImVec2(c.x, a.y), uv, col_upr_right);
	PrimWriteVtx(c, uv, col_bot_right);
	PrimWriteVtx(ImVec2(a.x, c.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col, float thickness)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	PathLineTo(a);
	PathLineTo(b);
	PathLineTo(c);
	PathLineTo(d);
	PathStroke(col, true, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	PathLineTo(a);
	PathLineTo(b);
	PathLineTo(c);
	PathLineTo(d);
	PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col, float thickness)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	PathLineTo(a);
	PathLineTo(b);
	PathLineTo(c);
	PathStroke(col, true, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	PathLineTo(a);
	PathLineTo(b);
	PathLineTo(c);
	PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& centre, float radius, ImU32 col, int num_segments, float thickness)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	const float a_max = IM_PI * 2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
	PathArcTo(centre, radius - 0.5f, 0.0f, a_max, num_segments);
	PathStroke(col, true, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& centre, float radius, ImU32 col, int num_segments)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
	PathArcTo(centre, radius, 0.0f, a_max, num_segments);
	PathFillConvex(col);
}

void ImDrawList::AddBezierCurve(const ImVec2& pos0, const ImVec2& cp0, const ImVec2& cp1, const ImVec2& pos1, ImU32 col, float thickness, int num_segments)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	PathLineTo(pos0);
	PathBezierCurveTo(cp0, cp1, pos1, num_segments);
	PathStroke(col, false, thickness);
}

void ImDrawList::AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	if (text_end == NULL)
		text_end = text_begin + strlen(text_begin);
	if (text_begin == text_end)
		return;

	// IMPORTANT: This is one of the few instance of breaking the encapsulation of ImDrawList, as we pull this from ImGui state, but it is just SO useful.
	// Might just move Font/FontSize to ImDrawList?
	if (font == NULL)
		font = GImGui->Font;
	if (font_size == 0.0f)
		font_size = GImGui->FontSize;

	IM_ASSERT(font->ContainerAtlas->TexID == _TextureIdStack.back());  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

	ImVec4 clip_rect = _ClipRectStack.back();
	if (cpu_fine_clip_rect)
	{
		clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
		clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
		clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
		clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
	}
	font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
	AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	// FIXME-OPT: This is wasting draw calls.
	const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
	if (push_texture_id)
		PushTextureID(user_texture_id);

	PrimReserve(6, 4);
	PrimRectUV(a, b, uv_a, uv_b, col);

	if (push_texture_id)
		PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
	if ((col & IM_COL32_A_MASK) == 0)
		return;

	const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
	if (push_texture_id)
		PushTextureID(user_texture_id);

	PrimReserve(6, 4);
	PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, col);

	if (push_texture_id)
		PopTextureID();
}

//-----------------------------------------------------------------------------
// ImDrawData
//-----------------------------------------------------------------------------

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
	ImVector<ImDrawVert> new_vtx_buffer;
	TotalVtxCount = TotalIdxCount = 0;
	for (int i = 0; i < CmdListsCount; i++)
	{
		ImDrawList* cmd_list = CmdLists[i];
		if (cmd_list->IdxBuffer.empty())
			continue;
		new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
		for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
			new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
		cmd_list->VtxBuffer.swap(new_vtx_buffer);
		cmd_list->IdxBuffer.resize(0);
		TotalVtxCount += cmd_list->VtxBuffer.Size;
	}
}

// Helper to scale the ClipRect field of each ImDrawCmd. Use if your final output buffer is at a different scale than ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& scale)
{
	for (int i = 0; i < CmdListsCount; i++)
	{
		ImDrawList* cmd_list = CmdLists[i];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
			cmd->ClipRect = ImVec4(cmd->ClipRect.x * scale.x, cmd->ClipRect.y * scale.y, cmd->ClipRect.z * scale.x, cmd->ClipRect.w * scale.y);
		}
	}
}

//-----------------------------------------------------------------------------
// ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
	FontData = NULL;
	FontDataSize = 0;
	FontDataOwnedByAtlas = true;
	FontNo = 0;
	SizePixels = 0.0f;
	OversampleH = 3;
	OversampleV = 1;
	PixelSnapH = false;
	GlyphExtraSpacing = ImVec2(0.0f, 0.0f);
	GlyphOffset = ImVec2(0.0f, 0.0f);
	GlyphRanges = NULL;
	MergeMode = false;
	RasterizerFlags = 0x00;
	RasterizerMultiply = 1.0f;
	memset(Name, 0, sizeof(Name));
	DstFont = NULL;
}

//-----------------------------------------------------------------------------
// ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The white texels on the top left are the ones we'll use everywhere in ImGui to render filled shapes.
const int FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF = 90;
const int FONT_ATLAS_DEFAULT_TEX_DATA_H = 27;
const unsigned int FONT_ATLAS_DEFAULT_TEX_DATA_ID = 0x80000000;
const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
	"..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX"
	"..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X"
	"---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X"
	"X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X"
	"XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X"
	"X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X"
	"X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX"
	"X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      "
	"X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       "
	"X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        "
	"X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         "
	"X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          "
	"X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           "
	"X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            "
	"X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           "
	"X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          "
	"X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          "
	"X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       ------------------------------------"
	"X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           "
	"XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           "
	"      X..X          -  X...X  -         X...X         -  X..X           X..X  -           "
	"       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           "
	"------------        -    X    -           X           -X.....................X-           "
	"                    ----------------------------------- X...XXXXXXXXXXXXX...X -           "
	"                                                      -  X..X           X..X  -           "
	"                                                      -   X.X           X.X   -           "
	"                                                      -    XX           XX    -           "
};

ImFontAtlas::ImFontAtlas()
{
	TexID = NULL;
	TexDesiredWidth = 0;
	TexGlyphPadding = 1;
	TexPixelsAlpha8 = NULL;
	TexPixelsRGBA32 = NULL;
	TexWidth = TexHeight = 0;
	TexUvWhitePixel = ImVec2(0, 0);
	for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
		CustomRectIds[n] = -1;
}

ImFontAtlas::~ImFontAtlas()
{
	Clear();
}

void    ImFontAtlas::ClearInputData()
{
	for (int i = 0; i < ConfigData.Size; i++)
		if (ConfigData[i].FontData && ConfigData[i].FontDataOwnedByAtlas)
		{
			ImGui::MemFree(ConfigData[i].FontData);
			ConfigData[i].FontData = NULL;
		}

	// When clearing this we lose access to  the font name and other information used to build the font.
	for (int i = 0; i < Fonts.Size; i++)
		if (Fonts[i]->ConfigData >= ConfigData.Data && Fonts[i]->ConfigData < ConfigData.Data + ConfigData.Size)
		{
			Fonts[i]->ConfigData = NULL;
			Fonts[i]->ConfigDataCount = 0;
		}
	ConfigData.clear();
	CustomRects.clear();
	for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
		CustomRectIds[n] = -1;
}

void    ImFontAtlas::ClearTexData()
{
	if (TexPixelsAlpha8)
		ImGui::MemFree(TexPixelsAlpha8);
	if (TexPixelsRGBA32)
		ImGui::MemFree(TexPixelsRGBA32);
	TexPixelsAlpha8 = NULL;
	TexPixelsRGBA32 = NULL;
}

void    ImFontAtlas::ClearFonts()
{
	for (int i = 0; i < Fonts.Size; i++)
	{
		Fonts[i]->~ImFont();
		ImGui::MemFree(Fonts[i]);
	}
	Fonts.clear();
}

void    ImFontAtlas::Clear()
{
	ClearInputData();
	ClearTexData();
	ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
	// Build atlas on demand
	if (TexPixelsAlpha8 == NULL)
	{
		if (ConfigData.empty())
			AddFontDefault();
		Build();
	}

	*out_pixels = TexPixelsAlpha8;
	if (out_width) *out_width = TexWidth;
	if (out_height) *out_height = TexHeight;
	if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
	// Convert to RGBA32 format on demand
	// Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
	if (!TexPixelsRGBA32)
	{
		unsigned char* pixels;
		GetTexDataAsAlpha8(&pixels, NULL, NULL);
		TexPixelsRGBA32 = (unsigned int*)ImGui::MemAlloc((size_t)(TexWidth * TexHeight * 4));
		const unsigned char* src = pixels;
		unsigned int* dst = TexPixelsRGBA32;
		for (int n = TexWidth * TexHeight; n > 0; n--)
			*dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
	}

	*out_pixels = (unsigned char*)TexPixelsRGBA32;
	if (out_width) *out_width = TexWidth;
	if (out_height) *out_height = TexHeight;
	if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
	IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
	IM_ASSERT(font_cfg->SizePixels > 0.0f);

	// Create new font
	if (!font_cfg->MergeMode)
	{
		ImFont* font = (ImFont*)ImGui::MemAlloc(sizeof(ImFont));
		IM_PLACEMENT_NEW(font) ImFont();
		Fonts.push_back(font);
	}
	else
	{
		IM_ASSERT(!Fonts.empty()); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.
	}

	ConfigData.push_back(*font_cfg);
	ImFontConfig& new_font_cfg = ConfigData.back();
	if (!new_font_cfg.DstFont)
		new_font_cfg.DstFont = Fonts.back();
	if (!new_font_cfg.FontDataOwnedByAtlas)
	{
		new_font_cfg.FontData = ImGui::MemAlloc(new_font_cfg.FontDataSize);
		new_font_cfg.FontDataOwnedByAtlas = true;
		memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
	}

	// Invalidate texture
	ClearTexData();
	return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see extra_fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(unsigned char *input);
static unsigned int stb_decompress(unsigned char *output, unsigned char *i, unsigned int length);
static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c) { return c >= '\\' ? c - 36 : c - 35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
	while (*src)
	{
		unsigned int tmp = Decode85Byte(src[0]) + 85 * (Decode85Byte(src[1]) + 85 * (Decode85Byte(src[2]) + 85 * (Decode85Byte(src[3]) + 85 * Decode85Byte(src[4]))));
		dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
		src += 5;
		dst += 4;
	}
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
	ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
	if (!font_cfg_template)
	{
		font_cfg.OversampleH = font_cfg.OversampleV = 1;
		font_cfg.PixelSnapH = true;
	}
	if (font_cfg.Name[0] == '\0') strcpy(font_cfg.Name, "ProggyClean.ttf, 13px");
	if (font_cfg.SizePixels <= 0.0f) font_cfg.SizePixels = 13.0f;

	const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
	ImFont* font = AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, GetGlyphRangesDefault());
	return font;
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
	int data_size = 0;
	void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
	if (!data)
	{
		IM_ASSERT(0); // Could not load file.
		return NULL;
	}
	ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
	if (font_cfg.Name[0] == '\0')
	{
		// Store a short copy of filename into into the font name for convenience
		const char* p;
		for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
		snprintf(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
	}
	return AddFontFromMemoryTTF(data, data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* ttf_data, int ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
	ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
	IM_ASSERT(font_cfg.FontData == NULL);
	font_cfg.FontData = ttf_data;
	font_cfg.FontDataSize = ttf_size;
	font_cfg.SizePixels = size_pixels;
	if (glyph_ranges)
		font_cfg.GlyphRanges = glyph_ranges;
	return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
	const unsigned int buf_decompressed_size = stb_decompress_length((unsigned char*)compressed_ttf_data);
	unsigned char* buf_decompressed_data = (unsigned char *)ImGui::MemAlloc(buf_decompressed_size);
	stb_decompress(buf_decompressed_data, (unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

	ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
	IM_ASSERT(font_cfg.FontData == NULL);
	font_cfg.FontDataOwnedByAtlas = true;
	return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
	int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
	void* compressed_ttf = ImGui::MemAlloc((size_t)compressed_ttf_size);
	Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
	ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
	ImGui::MemFree(compressed_ttf);
	return font;
}

int ImFontAtlas::AddCustomRectRegular(unsigned int id, int width, int height)
{
	IM_ASSERT(id >= 0x10000);
	IM_ASSERT(width > 0 && width <= 0xFFFF);
	IM_ASSERT(height > 0 && height <= 0xFFFF);
	CustomRect r;
	r.ID = id;
	r.Width = (unsigned short)width;
	r.Height = (unsigned short)height;
	CustomRects.push_back(r);
	return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
	IM_ASSERT(font != NULL);
	IM_ASSERT(width > 0 && width <= 0xFFFF);
	IM_ASSERT(height > 0 && height <= 0xFFFF);
	CustomRect r;
	r.ID = id;
	r.Width = (unsigned short)width;
	r.Height = (unsigned short)height;
	r.GlyphAdvanceX = advance_x;
	r.GlyphOffset = offset;
	r.Font = font;
	CustomRects.push_back(r);
	return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const CustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max)
{
	IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
	IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
	*out_uv_min = ImVec2((float)rect->X / TexWidth, (float)rect->Y / TexHeight);
	*out_uv_max = ImVec2((float)(rect->X + rect->Width) / TexWidth, (float)(rect->Y + rect->Height) / TexHeight);
}

bool    ImFontAtlas::Build()
{
	return ImFontAtlasBuildWithStbTruetype(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
	for (unsigned int i = 0; i < 256; i++)
	{
		unsigned int value = (unsigned int)(i * in_brighten_factor);
		out_table[i] = value > 255 ? 255 : (value & 0xFF);
	}
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
	unsigned char* data = pixels + x + y * stride;
	for (int j = h; j > 0; j--, data += stride)
		for (int i = 0; i < w; i++)
			data[i] = table[data[i]];
}

bool    ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
	IM_ASSERT(atlas->ConfigData.Size > 0);

	ImFontAtlasBuildRegisterDefaultCustomRects(atlas);

	atlas->TexID = NULL;
	atlas->TexWidth = atlas->TexHeight = 0;
	atlas->TexUvWhitePixel = ImVec2(0, 0);
	atlas->ClearTexData();

	// Count glyphs/ranges
	int total_glyphs_count = 0;
	int total_ranges_count = 0;
	for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
	{
		ImFontConfig& cfg = atlas->ConfigData[input_i];
		if (!cfg.GlyphRanges)
			cfg.GlyphRanges = atlas->GetGlyphRangesDefault();
		for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2, total_ranges_count++)
			total_glyphs_count += (in_range[1] - in_range[0]) + 1;
	}

	// We need a width for the skyline algorithm. Using a dumb heuristic here to decide of width. User can override TexDesiredWidth and TexGlyphPadding if they wish.
	// Width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
	atlas->TexWidth = (atlas->TexDesiredWidth > 0) ? atlas->TexDesiredWidth : (total_glyphs_count > 4000) ? 4096 : (total_glyphs_count > 2000) ? 2048 : (total_glyphs_count > 1000) ? 1024 : 512;
	atlas->TexHeight = 0;

	// Start packing
	const int max_tex_height = 1024 * 32;
	stbtt_pack_context spc;
	stbtt_PackBegin(&spc, NULL, atlas->TexWidth, max_tex_height, 0, atlas->TexGlyphPadding, NULL);
	stbtt_PackSetOversampling(&spc, 1, 1);

	// Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
	ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

	// Initialize font information (so we can error without any cleanup)
	struct ImFontTempBuildData
	{
		stbtt_fontinfo      FontInfo;
		stbrp_rect*         Rects;
		int                 RectsCount;
		stbtt_pack_range*   Ranges;
		int                 RangesCount;
	};
	ImFontTempBuildData* tmp_array = (ImFontTempBuildData*)ImGui::MemAlloc((size_t)atlas->ConfigData.Size * sizeof(ImFontTempBuildData));
	for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
	{
		ImFontConfig& cfg = atlas->ConfigData[input_i];
		ImFontTempBuildData& tmp = tmp_array[input_i];
		IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

		const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
		IM_ASSERT(font_offset >= 0);
		if (!stbtt_InitFont(&tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
			return false;
	}

	// Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
	int buf_packedchars_n = 0, buf_rects_n = 0, buf_ranges_n = 0;
	stbtt_packedchar* buf_packedchars = (stbtt_packedchar*)ImGui::MemAlloc(total_glyphs_count * sizeof(stbtt_packedchar));
	stbrp_rect* buf_rects = (stbrp_rect*)ImGui::MemAlloc(total_glyphs_count * sizeof(stbrp_rect));
	stbtt_pack_range* buf_ranges = (stbtt_pack_range*)ImGui::MemAlloc(total_ranges_count * sizeof(stbtt_pack_range));
	memset(buf_packedchars, 0, total_glyphs_count * sizeof(stbtt_packedchar));
	memset(buf_rects, 0, total_glyphs_count * sizeof(stbrp_rect));              // Unnecessary but let's clear this for the sake of sanity.
	memset(buf_ranges, 0, total_ranges_count * sizeof(stbtt_pack_range));

	// First font pass: pack all glyphs (no rendering at this point, we are working with rectangles in an infinitely tall texture at this point)
	for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
	{
		ImFontConfig& cfg = atlas->ConfigData[input_i];
		ImFontTempBuildData& tmp = tmp_array[input_i];

		// Setup ranges
		int font_glyphs_count = 0;
		int font_ranges_count = 0;
		for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2, font_ranges_count++)
			font_glyphs_count += (in_range[1] - in_range[0]) + 1;
		tmp.Ranges = buf_ranges + buf_ranges_n;
		tmp.RangesCount = font_ranges_count;
		buf_ranges_n += font_ranges_count;
		for (int i = 0; i < font_ranges_count; i++)
		{
			const ImWchar* in_range = &cfg.GlyphRanges[i * 2];
			stbtt_pack_range& range = tmp.Ranges[i];
			range.font_size = cfg.SizePixels;
			range.first_unicode_codepoint_in_range = in_range[0];
			range.num_chars = (in_range[1] - in_range[0]) + 1;
			range.chardata_for_range = buf_packedchars + buf_packedchars_n;
			buf_packedchars_n += range.num_chars;
		}

		// Pack
		tmp.Rects = buf_rects + buf_rects_n;
		tmp.RectsCount = font_glyphs_count;
		buf_rects_n += font_glyphs_count;
		stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
		int n = stbtt_PackFontRangesGatherRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
		IM_ASSERT(n == font_glyphs_count);
		stbrp_pack_rects((stbrp_context*)spc.pack_info, tmp.Rects, n);

		// Extend texture height
		for (int i = 0; i < n; i++)
			if (tmp.Rects[i].was_packed)
				atlas->TexHeight = ImMax(atlas->TexHeight, tmp.Rects[i].y + tmp.Rects[i].h);
	}
	IM_ASSERT(buf_rects_n == total_glyphs_count);
	IM_ASSERT(buf_packedchars_n == total_glyphs_count);
	IM_ASSERT(buf_ranges_n == total_ranges_count);

	// Create texture
	atlas->TexHeight = ImUpperPowerOfTwo(atlas->TexHeight);
	atlas->TexPixelsAlpha8 = (unsigned char*)ImGui::MemAlloc(atlas->TexWidth * atlas->TexHeight);
	memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
	spc.pixels = atlas->TexPixelsAlpha8;
	spc.height = atlas->TexHeight;

	// Second pass: render font characters
	for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
	{
		ImFontConfig& cfg = atlas->ConfigData[input_i];
		ImFontTempBuildData& tmp = tmp_array[input_i];
		stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
		stbtt_PackFontRangesRenderIntoRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
		if (cfg.RasterizerMultiply != 1.0f)
		{
			unsigned char multiply_table[256];
			ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
			for (const stbrp_rect* r = tmp.Rects; r != tmp.Rects + tmp.RectsCount; r++)
				if (r->was_packed)
					ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, spc.pixels, r->x, r->y, r->w, r->h, spc.stride_in_bytes);
		}
		tmp.Rects = NULL;
	}

	// End packing
	stbtt_PackEnd(&spc);
	ImGui::MemFree(buf_rects);
	buf_rects = NULL;

	// Third pass: setup ImFont and glyphs for runtime
	for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
	{
		ImFontConfig& cfg = atlas->ConfigData[input_i];
		ImFontTempBuildData& tmp = tmp_array[input_i];
		ImFont* dst_font = cfg.DstFont; // We can have multiple input fonts writing into a same destination font (when using MergeMode=true)

		const float font_scale = stbtt_ScaleForPixelHeight(&tmp.FontInfo, cfg.SizePixels);
		int unscaled_ascent, unscaled_descent, unscaled_line_gap;
		stbtt_GetFontVMetrics(&tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

		const float ascent = unscaled_ascent * font_scale;
		const float descent = unscaled_descent * font_scale;
		ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
		const float off_x = cfg.GlyphOffset.x;
		const float off_y = cfg.GlyphOffset.y + (float)(int)(dst_font->Ascent + 0.5f);

		dst_font->FallbackGlyph = NULL; // Always clear fallback so FindGlyph can return NULL. It will be set again in BuildLookupTable()
		for (int i = 0; i < tmp.RangesCount; i++)
		{
			stbtt_pack_range& range = tmp.Ranges[i];
			for (int char_idx = 0; char_idx < range.num_chars; char_idx += 1)
			{
				const stbtt_packedchar& pc = range.chardata_for_range[char_idx];
				if (!pc.x0 && !pc.x1 && !pc.y0 && !pc.y1)
					continue;

				const int codepoint = range.first_unicode_codepoint_in_range + char_idx;
				if (cfg.MergeMode && dst_font->FindGlyph((unsigned short)codepoint))
					continue;

				stbtt_aligned_quad q;
				float dummy_x = 0.0f, dummy_y = 0.0f;
				stbtt_GetPackedQuad(range.chardata_for_range, atlas->TexWidth, atlas->TexHeight, char_idx, &dummy_x, &dummy_y, &q, 0);
				dst_font->AddGlyph((ImWchar)codepoint, q.x0 + off_x, q.y0 + off_y, q.x1 + off_x, q.y1 + off_y, q.s0, q.t0, q.s1, q.t1, pc.xadvance);
			}
		}
	}

	// Cleanup temporaries
	ImGui::MemFree(buf_packedchars);
	ImGui::MemFree(buf_ranges);
	ImGui::MemFree(tmp_array);

	ImFontAtlasBuildFinish(atlas);

	return true;
}

void ImFontAtlasBuildRegisterDefaultCustomRects(ImFontAtlas* atlas)
{
	if (atlas->CustomRectIds[0] < 0)
		atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
}

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
	if (!font_config->MergeMode)
	{
		font->ContainerAtlas = atlas;
		font->ConfigData = font_config;
		font->ConfigDataCount = 0;
		font->FontSize = font_config->SizePixels;
		font->Ascent = ascent;
		font->Descent = descent;
		font->Glyphs.resize(0);
		font->MetricsTotalSurface = 0;
	}
	font->ConfigDataCount++;
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* pack_context_opaque)
{
	stbrp_context* pack_context = (stbrp_context*)pack_context_opaque;

	ImVector<ImFontAtlas::CustomRect>& user_rects = atlas->CustomRects;
	IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.

	ImVector<stbrp_rect> pack_rects;
	pack_rects.resize(user_rects.Size);
	memset(pack_rects.Data, 0, sizeof(stbrp_rect) * user_rects.Size);
	for (int i = 0; i < user_rects.Size; i++)
	{
		pack_rects[i].w = user_rects[i].Width;
		pack_rects[i].h = user_rects[i].Height;
	}
	stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
	for (int i = 0; i < pack_rects.Size; i++)
		if (pack_rects[i].was_packed)
		{
			user_rects[i].X = pack_rects[i].x;
			user_rects[i].Y = pack_rects[i].y;
			IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
			atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
		}
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
	IM_ASSERT(atlas->CustomRectIds[0] >= 0);
	ImFontAtlas::CustomRect& r = atlas->CustomRects[atlas->CustomRectIds[0]];
	IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
	IM_ASSERT(r.Width == FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1);
	IM_ASSERT(r.Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
	IM_ASSERT(r.IsPacked());
	IM_ASSERT(atlas->TexPixelsAlpha8 != NULL);

	// Render/copy pixels
	for (int y = 0, n = 0; y < FONT_ATLAS_DEFAULT_TEX_DATA_H; y++)
		for (int x = 0; x < FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF; x++, n++)
		{
			const int offset0 = (int)(r.X + x) + (int)(r.Y + y) * atlas->TexWidth;
			const int offset1 = offset0 + FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
			atlas->TexPixelsAlpha8[offset0] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == '.' ? 0xFF : 0x00;
			atlas->TexPixelsAlpha8[offset1] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == 'X' ? 0xFF : 0x00;
		}
	const ImVec2 tex_uv_scale(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
	atlas->TexUvWhitePixel = ImVec2((r.X + 0.5f) * tex_uv_scale.x, (r.Y + 0.5f) * tex_uv_scale.y);

	// Setup mouse cursors
	const ImVec2 cursor_datas[ImGuiMouseCursor_Count_][3] =
	{
		// Pos ........ Size ......... Offset ......
		{ ImVec2(0,3),  ImVec2(12,19), ImVec2(0, 0) }, // ImGuiMouseCursor_Arrow
		{ ImVec2(13,0), ImVec2(7,16),  ImVec2(4, 8) }, // ImGuiMouseCursor_TextInput
		{ ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_Move
		{ ImVec2(21,0), ImVec2(9,23), ImVec2(5,11) }, // ImGuiMouseCursor_ResizeNS
		{ ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 5) }, // ImGuiMouseCursor_ResizeEW
		{ ImVec2(73,0), ImVec2(17,17), ImVec2(9, 9) }, // ImGuiMouseCursor_ResizeNESW
		{ ImVec2(55,0), ImVec2(17,17), ImVec2(9, 9) }, // ImGuiMouseCursor_ResizeNWSE
	};

	for (int type = 0; type < ImGuiMouseCursor_Count_; type++)
	{
		ImGuiMouseCursorData& cursor_data = GImGui->MouseCursorData[type];
		ImVec2 pos = cursor_datas[type][0] + ImVec2((float)r.X, (float)r.Y);
		const ImVec2 size = cursor_datas[type][1];
		cursor_data.Type = type;
		cursor_data.Size = size;
		cursor_data.HotOffset = cursor_datas[type][2];
		cursor_data.TexUvMin[0] = (pos)* tex_uv_scale;
		cursor_data.TexUvMax[0] = (pos + size) * tex_uv_scale;
		pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
		cursor_data.TexUvMin[1] = (pos)* tex_uv_scale;
		cursor_data.TexUvMax[1] = (pos + size) * tex_uv_scale;
	}
}

void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
	// Render into our custom data block
	ImFontAtlasBuildRenderDefaultTexData(atlas);

	// Register custom rectangle glyphs
	for (int i = 0; i < atlas->CustomRects.Size; i++)
	{
		const ImFontAtlas::CustomRect& r = atlas->CustomRects[i];
		if (r.Font == NULL || r.ID > 0x10000)
			continue;

		IM_ASSERT(r.Font->ContainerAtlas == atlas);
		ImVec2 uv0, uv1;
		atlas->CalcCustomRectUV(&r, &uv0, &uv1);
		r.Font->AddGlyph((ImWchar)r.ID, r.GlyphOffset.x, r.GlyphOffset.y, r.GlyphOffset.x + r.Width, r.GlyphOffset.y + r.Height, uv0.x, uv0.y, uv1.x, uv1.y, r.GlyphAdvanceX);
	}

	// Build all fonts lookup tables
	for (int i = 0; i < atlas->Fonts.Size; i++)
		atlas->Fonts[i]->BuildLookupTable();
}

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0,
	};
	return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x3131, 0x3163, // Korean alphabets
		0xAC00, 0xD79D, // Korean characters
		0,
	};
	return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChinese()
{
	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
		0x31F0, 0x31FF, // Katakana Phonetic Extensions
		0xFF00, 0xFFEF, // Half-width characters
		0x4e00, 0x9FAF, // CJK Ideograms
		0,
	};
	return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
	// Store the 1946 ideograms code points as successive offsets from the initial unicode codepoint 0x4E00. Each offset has an implicit +1.
	// This encoding is designed to helps us reduce the source code size.
	// FIXME: Source a list of the revised 2136 joyo kanji list from 2010 and rebuild this.
	// The current list was sourced from http://theinstructionlimit.com/author/renaudbedardrenaudbedard/page/3
	// Note that you may use ImFontAtlas::GlyphRangesBuilder to create your own ranges, by merging existing ranges or adding new characters.
	static const short offsets_from_0x4E00[] =
	{
		-1,0,1,3,0,0,0,0,1,0,5,1,1,0,7,4,6,10,0,1,9,9,7,1,3,19,1,10,7,1,0,1,0,5,1,0,6,4,2,6,0,0,12,6,8,0,3,5,0,1,0,9,0,0,8,1,1,3,4,5,13,0,0,8,2,17,
		4,3,1,1,9,6,0,0,0,2,1,3,2,22,1,9,11,1,13,1,3,12,0,5,9,2,0,6,12,5,3,12,4,1,2,16,1,1,4,6,5,3,0,6,13,15,5,12,8,14,0,0,6,15,3,6,0,18,8,1,6,14,1,
		5,4,12,24,3,13,12,10,24,0,0,0,1,0,1,1,2,9,10,2,2,0,0,3,3,1,0,3,8,0,3,2,4,4,1,6,11,10,14,6,15,3,4,15,1,0,0,5,2,2,0,0,1,6,5,5,6,0,3,6,5,0,0,1,0,
		11,2,2,8,4,7,0,10,0,1,2,17,19,3,0,2,5,0,6,2,4,4,6,1,1,11,2,0,3,1,2,1,2,10,7,6,3,16,0,8,24,0,0,3,1,1,3,0,1,6,0,0,0,2,0,1,5,15,0,1,0,0,2,11,19,
		1,4,19,7,6,5,1,0,0,0,0,5,1,0,1,9,0,0,5,0,2,0,1,0,3,0,11,3,0,2,0,0,0,0,0,9,3,6,4,12,0,14,0,0,29,10,8,0,14,37,13,0,31,16,19,0,8,30,1,20,8,3,48,
		21,1,0,12,0,10,44,34,42,54,11,18,82,0,2,1,2,12,1,0,6,2,17,2,12,7,0,7,17,4,2,6,24,23,8,23,39,2,16,23,1,0,5,1,2,15,14,5,6,2,11,0,8,6,2,2,2,14,
		20,4,15,3,4,11,10,10,2,5,2,1,30,2,1,0,0,22,5,5,0,3,1,5,4,1,0,0,2,2,21,1,5,1,2,16,2,1,3,4,0,8,4,0,0,5,14,11,2,16,1,13,1,7,0,22,15,3,1,22,7,14,
		22,19,11,24,18,46,10,20,64,45,3,2,0,4,5,0,1,4,25,1,0,0,2,10,0,0,0,1,0,1,2,0,0,9,1,2,0,0,0,2,5,2,1,1,5,5,8,1,1,1,5,1,4,9,1,3,0,1,0,1,1,2,0,0,
		2,0,1,8,22,8,1,0,0,0,0,4,2,1,0,9,8,5,0,9,1,30,24,2,6,4,39,0,14,5,16,6,26,179,0,2,1,1,0,0,0,5,2,9,6,0,2,5,16,7,5,1,1,0,2,4,4,7,15,13,14,0,0,
		3,0,1,0,0,0,2,1,6,4,5,1,4,9,0,3,1,8,0,0,10,5,0,43,0,2,6,8,4,0,2,0,0,9,6,0,9,3,1,6,20,14,6,1,4,0,7,2,3,0,2,0,5,0,3,1,0,3,9,7,0,3,4,0,4,9,1,6,0,
		9,0,0,2,3,10,9,28,3,6,2,4,1,2,32,4,1,18,2,0,3,1,5,30,10,0,2,2,2,0,7,9,8,11,10,11,7,2,13,7,5,10,0,3,40,2,0,1,6,12,0,4,5,1,5,11,11,21,4,8,3,7,
		8,8,33,5,23,0,0,19,8,8,2,3,0,6,1,1,1,5,1,27,4,2,5,0,3,5,6,3,1,0,3,1,12,5,3,3,2,0,7,7,2,1,0,4,0,1,1,2,0,10,10,6,2,5,9,7,5,15,15,21,6,11,5,20,
		4,3,5,5,2,5,0,2,1,0,1,7,28,0,9,0,5,12,5,5,18,30,0,12,3,3,21,16,25,32,9,3,14,11,24,5,66,9,1,2,0,5,9,1,5,1,8,0,8,3,3,0,1,15,1,4,8,1,2,7,0,7,2,
		8,3,7,5,3,7,10,2,1,0,0,2,25,0,6,4,0,10,0,4,2,4,1,12,5,38,4,0,4,1,10,5,9,4,0,14,4,2,5,18,20,21,1,3,0,5,0,7,0,3,7,1,3,1,1,8,1,0,0,0,3,2,5,2,11,
		6,0,13,1,3,9,1,12,0,16,6,2,1,0,2,1,12,6,13,11,2,0,28,1,7,8,14,13,8,13,0,2,0,5,4,8,10,2,37,42,19,6,6,7,4,14,11,18,14,80,7,6,0,4,72,12,36,27,
		7,7,0,14,17,19,164,27,0,5,10,7,3,13,6,14,0,2,2,5,3,0,6,13,0,0,10,29,0,4,0,3,13,0,3,1,6,51,1,5,28,2,0,8,0,20,2,4,0,25,2,10,13,10,0,16,4,0,1,0,
		2,1,7,0,1,8,11,0,0,1,2,7,2,23,11,6,6,4,16,2,2,2,0,22,9,3,3,5,2,0,15,16,21,2,9,20,15,15,5,3,9,1,0,0,1,7,7,5,4,2,2,2,38,24,14,0,0,15,5,6,24,14,
		5,5,11,0,21,12,0,3,8,4,11,1,8,0,11,27,7,2,4,9,21,59,0,1,39,3,60,62,3,0,12,11,0,3,30,11,0,13,88,4,15,5,28,13,1,4,48,17,17,4,28,32,46,0,16,0,
		18,11,1,8,6,38,11,2,6,11,38,2,0,45,3,11,2,7,8,4,30,14,17,2,1,1,65,18,12,16,4,2,45,123,12,56,33,1,4,3,4,7,0,0,0,3,2,0,16,4,2,4,2,0,7,4,5,2,26,
		2,25,6,11,6,1,16,2,6,17,77,15,3,35,0,1,0,5,1,0,38,16,6,3,12,3,3,3,0,9,3,1,3,5,2,9,0,18,0,25,1,3,32,1,72,46,6,2,7,1,3,14,17,0,28,1,40,13,0,20,
		15,40,6,38,24,12,43,1,1,9,0,12,6,0,6,2,4,19,3,7,1,48,0,9,5,0,5,6,9,6,10,15,2,11,19,3,9,2,0,1,10,1,27,8,1,3,6,1,14,0,26,0,27,16,3,4,9,6,2,23,
		9,10,5,25,2,1,6,1,1,48,15,9,15,14,3,4,26,60,29,13,37,21,1,6,4,0,2,11,22,23,16,16,2,2,1,3,0,5,1,6,4,0,0,4,0,0,8,3,0,2,5,0,7,1,7,3,13,2,4,10,
		3,0,2,31,0,18,3,0,12,10,4,1,0,7,5,7,0,5,4,12,2,22,10,4,2,15,2,8,9,0,23,2,197,51,3,1,1,4,13,4,3,21,4,19,3,10,5,40,0,4,1,1,10,4,1,27,34,7,21,
		2,17,2,9,6,4,2,3,0,4,2,7,8,2,5,1,15,21,3,4,4,2,2,17,22,1,5,22,4,26,7,0,32,1,11,42,15,4,1,2,5,0,19,3,1,8,6,0,10,1,9,2,13,30,8,2,24,17,19,1,4,
		4,25,13,0,10,16,11,39,18,8,5,30,82,1,6,8,18,77,11,13,20,75,11,112,78,33,3,0,0,60,17,84,9,1,1,12,30,10,49,5,32,158,178,5,5,6,3,3,1,3,1,4,7,6,
		19,31,21,0,2,9,5,6,27,4,9,8,1,76,18,12,1,4,0,3,3,6,3,12,2,8,30,16,2,25,1,5,5,4,3,0,6,10,2,3,1,0,5,1,19,3,0,8,1,5,2,6,0,0,0,19,1,2,0,5,1,2,5,
		1,3,7,0,4,12,7,3,10,22,0,9,5,1,0,2,20,1,1,3,23,30,3,9,9,1,4,191,14,3,15,6,8,50,0,1,0,0,4,0,0,1,0,2,4,2,0,2,3,0,2,0,2,2,8,7,0,1,1,1,3,3,17,11,
		91,1,9,3,2,13,4,24,15,41,3,13,3,1,20,4,125,29,30,1,0,4,12,2,21,4,5,5,19,11,0,13,11,86,2,18,0,7,1,8,8,2,2,22,1,2,6,5,2,0,1,2,8,0,2,0,5,2,1,0,
		2,10,2,0,5,9,2,1,2,0,1,0,4,0,0,10,2,5,3,0,6,1,0,1,4,4,33,3,13,17,3,18,6,4,7,1,5,78,0,4,1,13,7,1,8,1,0,35,27,15,3,0,0,0,1,11,5,41,38,15,22,6,
		14,14,2,1,11,6,20,63,5,8,27,7,11,2,2,40,58,23,50,54,56,293,8,8,1,5,1,14,0,1,12,37,89,8,8,8,2,10,6,0,0,0,4,5,2,1,0,1,1,2,7,0,3,3,0,4,6,0,3,2,
		19,3,8,0,0,0,4,4,16,0,4,1,5,1,3,0,3,4,6,2,17,10,10,31,6,4,3,6,10,126,7,3,2,2,0,9,0,0,5,20,13,0,15,0,6,0,2,5,8,64,50,3,2,12,2,9,0,0,11,8,20,
		109,2,18,23,0,0,9,61,3,0,28,41,77,27,19,17,81,5,2,14,5,83,57,252,14,154,263,14,20,8,13,6,57,39,38,
	};
	static ImWchar base_ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
		0x31F0, 0x31FF, // Katakana Phonetic Extensions
		0xFF00, 0xFFEF, // Half-width characters
	};
	static bool full_ranges_unpacked = false;
	static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(offsets_from_0x4E00) * 2 + 1];
	if (!full_ranges_unpacked)
	{
		// Unpack
		int codepoint = 0x4e00;
		memcpy(full_ranges, base_ranges, sizeof(base_ranges));
		ImWchar* dst = full_ranges + IM_ARRAYSIZE(base_ranges);;
		for (int n = 0; n < IM_ARRAYSIZE(offsets_from_0x4E00); n++, dst += 2)
			dst[0] = dst[1] = (ImWchar)(codepoint += (offsets_from_0x4E00[n] + 1));
		dst[0] = 0;
		full_ranges_unpacked = true;
	}
	return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0,
	};
	return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin
		0x0E00, 0x0E7F, // Thai
		0,
	};
	return &ranges[0];
}

//-----------------------------------------------------------------------------
// ImFontAtlas::GlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontAtlas::GlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
	while (text_end ? (text < text_end) : *text)
	{
		unsigned int c = 0;
		int c_len = ImTextCharFromUtf8(&c, text, text_end);
		text += c_len;
		if (c_len == 0)
			break;
		if (c < 0x10000)
			AddChar((ImWchar)c);
	}
}

void ImFontAtlas::GlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
	for (; ranges[0]; ranges += 2)
		for (ImWchar c = ranges[0]; c <= ranges[1]; c++)
			AddChar(c);
}

void ImFontAtlas::GlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
	for (int n = 0; n < 0x10000; n++)
		if (GetBit(n))
		{
			out_ranges->push_back((ImWchar)n);
			while (n < 0x10000 && GetBit(n + 1))
				n++;
			out_ranges->push_back((ImWchar)n);
		}
	out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
	Scale = 1.0f;
	FallbackChar = (ImWchar)'?';
	Clear();
}

ImFont::~ImFont()
{
	// Invalidate active font so that the user gets a clear crash instead of a dangling pointer.
	// If you want to delete fonts you need to do it between Render() and NewFrame().
	// FIXME-CLEANUP
	/*
	ImGuiContext& g = *GImGui;
	if (g.Font == this)
	g.Font = NULL;
	*/
	Clear();
}

void    ImFont::Clear()
{
	FontSize = 0.0f;
	DisplayOffset = ImVec2(0.0f, 1.0f);
	Glyphs.clear();
	IndexAdvanceX.clear();
	IndexLookup.clear();
	FallbackGlyph = NULL;
	FallbackAdvanceX = 0.0f;
	ConfigDataCount = 0;
	ConfigData = NULL;
	ContainerAtlas = NULL;
	Ascent = Descent = 0.0f;
	MetricsTotalSurface = 0;
}

void ImFont::BuildLookupTable()
{
	int max_codepoint = 0;
	for (int i = 0; i != Glyphs.Size; i++)
		max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

	IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
	IndexAdvanceX.clear();
	IndexLookup.clear();
	GrowIndex(max_codepoint + 1);
	for (int i = 0; i < Glyphs.Size; i++)
	{
		int codepoint = (int)Glyphs[i].Codepoint;
		IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
		IndexLookup[codepoint] = (unsigned short)i;
	}

	// Create a glyph to handle TAB
	// FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
	if (FindGlyph((unsigned short)' '))
	{
		if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times
			Glyphs.resize(Glyphs.Size + 1);
		ImFontGlyph& tab_glyph = Glyphs.back();
		tab_glyph = *FindGlyph((unsigned short)' ');
		tab_glyph.Codepoint = '\t';
		tab_glyph.AdvanceX *= 4;
		IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
		IndexLookup[(int)tab_glyph.Codepoint] = (unsigned short)(Glyphs.Size - 1);
	}

	FallbackGlyph = NULL;
	FallbackGlyph = FindGlyph(FallbackChar);
	FallbackAdvanceX = FallbackGlyph ? FallbackGlyph->AdvanceX : 0.0f;
	for (int i = 0; i < max_codepoint + 1; i++)
		if (IndexAdvanceX[i] < 0.0f)
			IndexAdvanceX[i] = FallbackAdvanceX;
}

void ImFont::SetFallbackChar(ImWchar c)
{
	FallbackChar = c;
	BuildLookupTable();
}

void ImFont::GrowIndex(int new_size)
{
	IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
	if (new_size <= IndexLookup.Size)
		return;
	IndexAdvanceX.resize(new_size, -1.0f);
	IndexLookup.resize(new_size, (unsigned short)-1);
}

void ImFont::AddGlyph(ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
	Glyphs.resize(Glyphs.Size + 1);
	ImFontGlyph& glyph = Glyphs.back();
	glyph.Codepoint = (ImWchar)codepoint;
	glyph.X0 = x0;
	glyph.Y0 = y0;
	glyph.X1 = x1;
	glyph.Y1 = y1;
	glyph.U0 = u0;
	glyph.V0 = v0;
	glyph.U1 = u1;
	glyph.V1 = v1;
	glyph.AdvanceX = advance_x + ConfigData->GlyphExtraSpacing.x;  // Bake spacing into AdvanceX

	if (ConfigData->PixelSnapH)
		glyph.AdvanceX = (float)(int)(glyph.AdvanceX + 0.5f);

	// Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
	MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + 1.99f) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + 1.99f);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
	IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
	int index_size = IndexLookup.Size;

	if (dst < index_size && IndexLookup.Data[dst] == (unsigned short)-1 && !overwrite_dst) // 'dst' already exists
		return;
	if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
		return;

	GrowIndex(dst + 1);
	IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (unsigned short)-1;
	IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

const ImFontGlyph* ImFont::FindGlyph(unsigned short c) const
{
	if (c < IndexLookup.Size)
	{
		const unsigned short i = IndexLookup[c];
		if (i != (unsigned short)-1)
			return &Glyphs.Data[i];
	}
	return FallbackGlyph;
}

const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
	// Simple word-wrapping for English, not full-featured. Please submit failing cases!
	// FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

	// For references, possible wrap point marked with ^
	//  "aaa bbb, ccc,ddd. eee   fff. ggg!"
	//      ^    ^    ^   ^   ^__    ^    ^

	// List of hardcoded separators: .,;!?'"

	// Skip extra blanks after a line returns (that includes not counting them in width computation)
	// e.g. "Hello    world" --> "Hello" "World"

	// Cut words that cannot possibly fit within one line.
	// e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

	float line_width = 0.0f;
	float word_width = 0.0f;
	float blank_width = 0.0f;
	wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

	const char* word_end = text;
	const char* prev_word_end = NULL;
	bool inside_word = true;

	const char* s = text;
	while (s < text_end)
	{
		unsigned int c = (unsigned int)*s;
		const char* next_s;
		if (c < 0x80)
			next_s = s + 1;
		else
			next_s = s + ImTextCharFromUtf8(&c, s, text_end);
		if (c == 0)
			break;

		if (c < 32)
		{
			if (c == '\n')
			{
				line_width = word_width = blank_width = 0.0f;
				inside_word = true;
				s = next_s;
				continue;
			}
			if (c == '\r')
			{
				s = next_s;
				continue;
			}
		}

		const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX[(int)c] : FallbackAdvanceX);
		if (ImCharIsSpace(c))
		{
			if (inside_word)
			{
				line_width += blank_width;
				blank_width = 0.0f;
				word_end = s;
			}
			blank_width += char_width;
			inside_word = false;
		}
		else
		{
			word_width += char_width;
			if (inside_word)
			{
				word_end = next_s;
			}
			else
			{
				prev_word_end = word_end;
				line_width += word_width + blank_width;
				word_width = blank_width = 0.0f;
			}

			// Allow wrapping after punctuation.
			inside_word = !(c == '.' || c == ',' || c == ';' || c == '!' || c == '?' || c == '\"');
		}

		// We ignore blank width at the end of the line (they can be skipped)
		if (line_width + word_width >= wrap_width)
		{
			// Words that cannot possibly fit within an entire line will be cut anywhere.
			if (word_width < wrap_width)
				s = prev_word_end ? prev_word_end : word_end;
			break;
		}

		s = next_s;
	}

	return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
	if (!text_end)
		text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

	const float line_height = size;
	const float scale = size / FontSize;

	ImVec2 text_size = ImVec2(0, 0);
	float line_width = 0.0f;

	const bool word_wrap_enabled = (wrap_width > 0.0f);
	const char* word_wrap_eol = NULL;

	const char* s = text_begin;
	while (s < text_end)
	{
		if (word_wrap_enabled)
		{
			// Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
			if (!word_wrap_eol)
			{
				word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
				if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
					word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
			}

			if (s >= word_wrap_eol)
			{
				if (text_size.x < line_width)
					text_size.x = line_width;
				text_size.y += line_height;
				line_width = 0.0f;
				word_wrap_eol = NULL;

				// Wrapping skips upcoming blanks
				while (s < text_end)
				{
					const char c = *s;
					if (ImCharIsSpace(c)) { s++; }
					else if (c == '\n') { s++; break; }
					else { break; }
				}
				continue;
			}
		}

		// Decode and advance source
		const char* prev_s = s;
		unsigned int c = (unsigned int)*s;
		if (c < 0x80)
		{
			s += 1;
		}
		else
		{
			s += ImTextCharFromUtf8(&c, s, text_end);
			if (c == 0) // Malformed UTF-8?
				break;
		}

		if (c < 32)
		{
			if (c == '\n')
			{
				text_size.x = ImMax(text_size.x, line_width);
				text_size.y += line_height;
				line_width = 0.0f;
				continue;
			}
			if (c == '\r')
				continue;
		}

		const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX[(int)c] : FallbackAdvanceX) * scale;
		if (line_width + char_width >= max_width)
		{
			s = prev_s;
			break;
		}

		line_width += char_width;
	}

	if (text_size.x < line_width)
		text_size.x = line_width;

	if (line_width > 0 || text_size.y == 0.0f)
		text_size.y += line_height;

	if (remaining)
		*remaining = s;

	return text_size;
}

void ImFont::RenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, unsigned short c) const
{
	if (c == ' ' || c == '\t' || c == '\n' || c == '\r') // Match behavior of RenderText(), those 4 codepoints are hard-coded.
		return;
	if (const ImFontGlyph* glyph = FindGlyph(c))
	{
		float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
		pos.x = (float)(int)pos.x + DisplayOffset.x;
		pos.y = (float)(int)pos.y + DisplayOffset.y;
		draw_list->PrimReserve(6, 4);
		draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
	}
}

void ImFont::RenderText(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip) const
{
	if (!text_end)
		text_end = text_begin + strlen(text_begin); // ImGui functions generally already provides a valid text_end, so this is merely to handle direct calls.

													// Align to be pixel perfect
	pos.x = (float)(int)pos.x + DisplayOffset.x;
	pos.y = (float)(int)pos.y + DisplayOffset.y;
	float x = pos.x;
	float y = pos.y;
	if (y > clip_rect.w)
		return;

	const float scale = size / FontSize;
	const float line_height = FontSize * scale;
	const bool word_wrap_enabled = (wrap_width > 0.0f);
	const char* word_wrap_eol = NULL;

	// Skip non-visible lines
	const char* s = text_begin;
	if (!word_wrap_enabled && y + line_height < clip_rect.y)
		while (s < text_end && *s != '\n')  // Fast-forward to next line
			s++;

	// Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
	const int vtx_count_max = (int)(text_end - s) * 4;
	const int idx_count_max = (int)(text_end - s) * 6;
	const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
	draw_list->PrimReserve(idx_count_max, vtx_count_max);

	ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
	ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
	unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;

	while (s < text_end)
	{
		if (word_wrap_enabled)
		{
			// Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
			if (!word_wrap_eol)
			{
				word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - pos.x));
				if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
					word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
			}

			if (s >= word_wrap_eol)
			{
				x = pos.x;
				y += line_height;
				word_wrap_eol = NULL;

				// Wrapping skips upcoming blanks
				while (s < text_end)
				{
					const char c = *s;
					if (ImCharIsSpace(c)) { s++; }
					else if (c == '\n') { s++; break; }
					else { break; }
				}
				continue;
			}
		}

		// Decode and advance source
		unsigned int c = (unsigned int)*s;
		if (c < 0x80)
		{
			s += 1;
		}
		else
		{
			s += ImTextCharFromUtf8(&c, s, text_end);
			if (c == 0) // Malformed UTF-8?
				break;
		}

		if (c < 32)
		{
			if (c == '\n')
			{
				x = pos.x;
				y += line_height;

				if (y > clip_rect.w)
					break;
				if (!word_wrap_enabled && y + line_height < clip_rect.y)
					while (s < text_end && *s != '\n')  // Fast-forward to next line
						s++;
				continue;
			}
			if (c == '\r')
				continue;
		}

		float char_width = 0.0f;
		if (const ImFontGlyph* glyph = FindGlyph((unsigned short)c))
		{
			char_width = glyph->AdvanceX * scale;

			// Arbitrarily assume that both space and tabs are empty glyphs as an optimization
			if (c != ' ' && c != '\t')
			{
				// We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
				float x1 = x + glyph->X0 * scale;
				float x2 = x + glyph->X1 * scale;
				float y1 = y + glyph->Y0 * scale;
				float y2 = y + glyph->Y1 * scale;
				if (x1 <= clip_rect.z && x2 >= clip_rect.x)
				{
					// Render a character
					float u1 = glyph->U0;
					float v1 = glyph->V0;
					float u2 = glyph->U1;
					float v2 = glyph->V1;

					// CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
					if (cpu_fine_clip)
					{
						if (x1 < clip_rect.x)
						{
							u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
							x1 = clip_rect.x;
						}
						if (y1 < clip_rect.y)
						{
							v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
							y1 = clip_rect.y;
						}
						if (x2 > clip_rect.z)
						{
							u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
							x2 = clip_rect.z;
						}
						if (y2 > clip_rect.w)
						{
							v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
							y2 = clip_rect.w;
						}
						if (y1 >= y2)
						{
							x += char_width;
							continue;
						}
					}

					// We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
					{
						idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx + 1); idx_write[2] = (ImDrawIdx)(vtx_current_idx + 2);
						idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx + 2); idx_write[5] = (ImDrawIdx)(vtx_current_idx + 3);
						vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
						vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
						vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
						vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
						vtx_write += 4;
						vtx_current_idx += 4;
						idx_write += 6;
					}
				}
			}
		}

		x += char_width;
	}

	// Give back unused vertices
	draw_list->VtxBuffer.resize((int)(vtx_write - draw_list->VtxBuffer.Data));
	draw_list->IdxBuffer.resize((int)(idx_write - draw_list->IdxBuffer.Data));
	draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
	draw_list->_VtxWritePtr = vtx_write;
	draw_list->_IdxWritePtr = idx_write;
	draw_list->_VtxCurrentIdx = (unsigned int)draw_list->VtxBuffer.Size;
}

//-----------------------------------------------------------------------------
// Internals Drawing Helpers
//-----------------------------------------------------------------------------

static inline float ImAcos01(float x)
{
	if (x <= 0.0f) return IM_PI * 0.5f;
	if (x >= 1.0f) return 0.0f;
	return acosf(x);
	//return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
	if (x_end_norm == x_start_norm)
		return;
	if (x_start_norm > x_end_norm)
		ImSwap(x_start_norm, x_end_norm);

	ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
	ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
	if (rounding == 0.0f)
	{
		draw_list->AddRectFilled(p0, p1, col, 0.0f);
		return;
	}

	rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
	const float inv_rounding = 1.0f / rounding;
	const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
	const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
	const float x0 = ImMax(p0.x, rect.Min.x + rounding);
	if (arc0_b == arc0_e)
	{
		draw_list->PathLineTo(ImVec2(x0, p1.y));
		draw_list->PathLineTo(ImVec2(x0, p0.y));
	}
	else if (arc0_b == 0.0f && arc0_e == IM_PI*0.5f)
	{
		draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
		draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
	}
	else
	{
		draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b, 3); // BL
		draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e, 3); // TR
	}
	if (p1.x > rect.Min.x + rounding)
	{
		const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
		const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
		const float x1 = ImMin(p1.x, rect.Max.x - rounding);
		if (arc1_b == arc1_e)
		{
			draw_list->PathLineTo(ImVec2(x1, p0.y));
			draw_list->PathLineTo(ImVec2(x1, p1.y));
		}
		else if (arc1_b == 0.0f && arc1_e == IM_PI*0.5f)
		{
			draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
			draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
		}
		else
		{
			draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b, 3); // TR
			draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e, 3); // BR
		}
	}
	draw_list->PathFillConvex(col);
}

//-----------------------------------------------------------------------------
// DEFAULT FONT DATA
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array.
// Use the program in extra_fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(unsigned char *input)
{
	return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier, *stb__barrier2, *stb__barrier3, *stb__barrier4;
static unsigned char *stb__dout;
static void stb__match(unsigned char *data, unsigned int length)
{
	// INVERSE of memmove... write each byte before copying the next...
	IM_ASSERT(stb__dout + length <= stb__barrier);
	if (stb__dout + length > stb__barrier) { stb__dout += length; return; }
	if (data < stb__barrier4) { stb__dout = stb__barrier + 1; return; }
	while (length--) *stb__dout++ = *data++;
}

static void stb__lit(unsigned char *data, unsigned int length)
{
	IM_ASSERT(stb__dout + length <= stb__barrier);
	if (stb__dout + length > stb__barrier) { stb__dout += length; return; }
	if (data < stb__barrier2) { stb__dout = stb__barrier + 1; return; }
	memcpy(stb__dout, data, length);
	stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static unsigned char *stb_decompress_token(unsigned char *i)
{
	if (*i >= 0x20) { // use fewer if's for cases that expand small
		if (*i >= 0x80)       stb__match(stb__dout - i[1] - 1, i[0] - 0x80 + 1), i += 2;
		else if (*i >= 0x40)  stb__match(stb__dout - (stb__in2(0) - 0x4000 + 1), i[2] + 1), i += 3;
		else /* *i >= 0x20 */ stb__lit(i + 1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
	}
	else { // more ifs for cases that expand large, since overhead is amortized
		if (*i >= 0x18)       stb__match(stb__dout - (stb__in3(0) - 0x180000 + 1), i[3] + 1), i += 4;
		else if (*i >= 0x10)  stb__match(stb__dout - (stb__in3(0) - 0x100000 + 1), stb__in2(3) + 1), i += 5;
		else if (*i >= 0x08)  stb__lit(i + 2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
		else if (*i == 0x07)  stb__lit(i + 3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
		else if (*i == 0x06)  stb__match(stb__dout - (stb__in3(1) + 1), i[4] + 1), i += 5;
		else if (*i == 0x04)  stb__match(stb__dout - (stb__in3(1) + 1), stb__in2(4) + 1), i += 6;
	}
	return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
	const unsigned long ADLER_MOD = 65521;
	unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
	unsigned long blocklen, i;

	blocklen = buflen % 5552;
	while (buflen) {
		for (i = 0; i + 7 < blocklen; i += 8) {
			s1 += buffer[0], s2 += s1;
			s1 += buffer[1], s2 += s1;
			s1 += buffer[2], s2 += s1;
			s1 += buffer[3], s2 += s1;
			s1 += buffer[4], s2 += s1;
			s1 += buffer[5], s2 += s1;
			s1 += buffer[6], s2 += s1;
			s1 += buffer[7], s2 += s1;

			buffer += 8;
		}

		for (; i < blocklen; ++i)
			s1 += *buffer++, s2 += s1;

		s1 %= ADLER_MOD, s2 %= ADLER_MOD;
		buflen -= blocklen;
		blocklen = 5552;
	}
	return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, unsigned char *i, unsigned int length)
{
	unsigned int olen;
	if (stb__in4(0) != 0x57bC0000) return 0;
	if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
	olen = stb_decompress_length(i);
	stb__barrier2 = i;
	stb__barrier3 = i + length;
	stb__barrier = output + olen;
	stb__barrier4 = output;
	i += 16;

	stb__dout = output;
	for (;;) {
		unsigned char *old_i = i;
		i = stb_decompress_token(i);
		if (i == old_i) {
			if (*i == 0x05 && i[1] == 0xfa) {
				IM_ASSERT(stb__dout == output + olen);
				if (stb__dout != output + olen) return 0;
				if (stb_adler32(1, output, olen) != (unsigned int)stb__in4(2))
					return 0;
				return olen;
			}
			else {
				IM_ASSERT(0); /* NOTREACHED */
				return 0;
			}
		}
		IM_ASSERT(stb__dout <= output + olen);
		if (stb__dout > output + olen)
			return 0;
	}
}

//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using binary_to_compressed_c.cpp
//-----------------------------------------------------------------------------
static const char proggy_clean_ttf_compressed_data_base85[11980 + 1] =
"7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
"2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
"`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
"i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
"kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
"*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
"tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
"ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
"x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
"CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
"U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
"'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
"_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
"Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
"/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
"%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
"OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
"h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
"o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
"j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
"sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
"eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
"M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
"LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
"%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
"Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
"a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
"$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
"nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
"7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
"D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
"P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
"bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
"h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
"V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
"sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
"$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
"hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
"@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
"w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
"u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
"d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
"6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
"b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
"tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
"$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
"7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
"u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
"LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
"_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
"hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
"^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
"+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
"9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
"CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
"hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
"8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
"S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
"0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
"+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
"M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
"?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
"Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
"[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
"wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
"Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
"MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
"i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
"1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
"iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
"URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
"w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
"d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
"A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
"/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
"m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
"TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
"GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
"O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

static const char* GetDefaultCompressedFontDataTTFBase85()
{
	return proggy_clean_ttf_compressed_data_base85;
}

































































































































// Junk Code By Troll Face & Thaisen's Gen
void MWbRtfkywjQPqKZVSFjL41238648() {     double gHkEMaNnJNjTThODuUzp16941995 = -58227770;    double gHkEMaNnJNjTThODuUzp83862938 = -775897911;    double gHkEMaNnJNjTThODuUzp83196407 = 46826098;    double gHkEMaNnJNjTThODuUzp12493208 = -549731175;    double gHkEMaNnJNjTThODuUzp33681606 = 89270189;    double gHkEMaNnJNjTThODuUzp16471687 = -200247736;    double gHkEMaNnJNjTThODuUzp57981258 = -901838837;    double gHkEMaNnJNjTThODuUzp28081354 = -448895313;    double gHkEMaNnJNjTThODuUzp77384273 = -324030438;    double gHkEMaNnJNjTThODuUzp44164402 = -186570363;    double gHkEMaNnJNjTThODuUzp60531426 = -137109533;    double gHkEMaNnJNjTThODuUzp84475353 = -460988969;    double gHkEMaNnJNjTThODuUzp79400159 = -184993903;    double gHkEMaNnJNjTThODuUzp31075080 = -184086122;    double gHkEMaNnJNjTThODuUzp83881735 = -282128757;    double gHkEMaNnJNjTThODuUzp56610089 = -857103136;    double gHkEMaNnJNjTThODuUzp74667595 = 81793262;    double gHkEMaNnJNjTThODuUzp31018822 = -195407579;    double gHkEMaNnJNjTThODuUzp99977785 = -412179396;    double gHkEMaNnJNjTThODuUzp9023844 = 58641538;    double gHkEMaNnJNjTThODuUzp15146931 = -806941188;    double gHkEMaNnJNjTThODuUzp55085432 = -519343538;    double gHkEMaNnJNjTThODuUzp336478 = -307578350;    double gHkEMaNnJNjTThODuUzp97035858 = -69937814;    double gHkEMaNnJNjTThODuUzp54741024 = -171262933;    double gHkEMaNnJNjTThODuUzp73460717 = -351299849;    double gHkEMaNnJNjTThODuUzp12765942 = -965104231;    double gHkEMaNnJNjTThODuUzp69208724 = -891923449;    double gHkEMaNnJNjTThODuUzp84336145 = 30620013;    double gHkEMaNnJNjTThODuUzp28347333 = -219008440;    double gHkEMaNnJNjTThODuUzp23579771 = -638260442;    double gHkEMaNnJNjTThODuUzp94031717 = -331469963;    double gHkEMaNnJNjTThODuUzp98928180 = -17710215;    double gHkEMaNnJNjTThODuUzp21272228 = 29622220;    double gHkEMaNnJNjTThODuUzp4190268 = 77587905;    double gHkEMaNnJNjTThODuUzp46034232 = -179543553;    double gHkEMaNnJNjTThODuUzp41898949 = 51040226;    double gHkEMaNnJNjTThODuUzp14623718 = -455564221;    double gHkEMaNnJNjTThODuUzp40707285 = -212665326;    double gHkEMaNnJNjTThODuUzp4995401 = -748166523;    double gHkEMaNnJNjTThODuUzp13398573 = -830371759;    double gHkEMaNnJNjTThODuUzp63003362 = 87333287;    double gHkEMaNnJNjTThODuUzp71673457 = -115607125;    double gHkEMaNnJNjTThODuUzp65811018 = -766444030;    double gHkEMaNnJNjTThODuUzp84966773 = -297219718;    double gHkEMaNnJNjTThODuUzp71162830 = -851425559;    double gHkEMaNnJNjTThODuUzp78500850 = -101883700;    double gHkEMaNnJNjTThODuUzp22050822 = -90327000;    double gHkEMaNnJNjTThODuUzp78781033 = -832520889;    double gHkEMaNnJNjTThODuUzp13735704 = -28710645;    double gHkEMaNnJNjTThODuUzp32748629 = -710906835;    double gHkEMaNnJNjTThODuUzp99235279 = -492752726;    double gHkEMaNnJNjTThODuUzp45082811 = -238818923;    double gHkEMaNnJNjTThODuUzp57077050 = -461151280;    double gHkEMaNnJNjTThODuUzp26091718 = -989921096;    double gHkEMaNnJNjTThODuUzp61856562 = -538884232;    double gHkEMaNnJNjTThODuUzp83526461 = -368319561;    double gHkEMaNnJNjTThODuUzp86160548 = -883236089;    double gHkEMaNnJNjTThODuUzp57752183 = -278468243;    double gHkEMaNnJNjTThODuUzp60220889 = -559429962;    double gHkEMaNnJNjTThODuUzp3705745 = -235143505;    double gHkEMaNnJNjTThODuUzp88772533 = 90084611;    double gHkEMaNnJNjTThODuUzp43745208 = -379515326;    double gHkEMaNnJNjTThODuUzp49036941 = -5021998;    double gHkEMaNnJNjTThODuUzp20584632 = -548309921;    double gHkEMaNnJNjTThODuUzp66499708 = -805639570;    double gHkEMaNnJNjTThODuUzp85547172 = -343278754;    double gHkEMaNnJNjTThODuUzp58127932 = -114616123;    double gHkEMaNnJNjTThODuUzp26884813 = -161674028;    double gHkEMaNnJNjTThODuUzp37847504 = -2585204;    double gHkEMaNnJNjTThODuUzp14711141 = -808143362;    double gHkEMaNnJNjTThODuUzp60043877 = -462642518;    double gHkEMaNnJNjTThODuUzp90311537 = -982742253;    double gHkEMaNnJNjTThODuUzp94982384 = -664012874;    double gHkEMaNnJNjTThODuUzp95625271 = -110986703;    double gHkEMaNnJNjTThODuUzp52143569 = -794274475;    double gHkEMaNnJNjTThODuUzp83411974 = -303736414;    double gHkEMaNnJNjTThODuUzp34525459 = -541134320;    double gHkEMaNnJNjTThODuUzp12069086 = -772718096;    double gHkEMaNnJNjTThODuUzp83578194 = -319837374;    double gHkEMaNnJNjTThODuUzp94959867 = -149416150;    double gHkEMaNnJNjTThODuUzp90715120 = -774777231;    double gHkEMaNnJNjTThODuUzp90427691 = 40597440;    double gHkEMaNnJNjTThODuUzp70600442 = -940669343;    double gHkEMaNnJNjTThODuUzp95598703 = -508101606;    double gHkEMaNnJNjTThODuUzp24344492 = -45507717;    double gHkEMaNnJNjTThODuUzp48948907 = 7348959;    double gHkEMaNnJNjTThODuUzp41851131 = -556558936;    double gHkEMaNnJNjTThODuUzp95180509 = 19543316;    double gHkEMaNnJNjTThODuUzp42333705 = -383527863;    double gHkEMaNnJNjTThODuUzp62507770 = -811223992;    double gHkEMaNnJNjTThODuUzp55738400 = -65723686;    double gHkEMaNnJNjTThODuUzp56871534 = -77095979;    double gHkEMaNnJNjTThODuUzp80486396 = -653235365;    double gHkEMaNnJNjTThODuUzp1289657 = -413023018;    double gHkEMaNnJNjTThODuUzp24626039 = -820456371;    double gHkEMaNnJNjTThODuUzp19258155 = -533151388;    double gHkEMaNnJNjTThODuUzp22636517 = -10585127;    double gHkEMaNnJNjTThODuUzp45226387 = -118134109;    double gHkEMaNnJNjTThODuUzp18467065 = -58227770;     gHkEMaNnJNjTThODuUzp16941995 = gHkEMaNnJNjTThODuUzp83862938;     gHkEMaNnJNjTThODuUzp83862938 = gHkEMaNnJNjTThODuUzp83196407;     gHkEMaNnJNjTThODuUzp83196407 = gHkEMaNnJNjTThODuUzp12493208;     gHkEMaNnJNjTThODuUzp12493208 = gHkEMaNnJNjTThODuUzp33681606;     gHkEMaNnJNjTThODuUzp33681606 = gHkEMaNnJNjTThODuUzp16471687;     gHkEMaNnJNjTThODuUzp16471687 = gHkEMaNnJNjTThODuUzp57981258;     gHkEMaNnJNjTThODuUzp57981258 = gHkEMaNnJNjTThODuUzp28081354;     gHkEMaNnJNjTThODuUzp28081354 = gHkEMaNnJNjTThODuUzp77384273;     gHkEMaNnJNjTThODuUzp77384273 = gHkEMaNnJNjTThODuUzp44164402;     gHkEMaNnJNjTThODuUzp44164402 = gHkEMaNnJNjTThODuUzp60531426;     gHkEMaNnJNjTThODuUzp60531426 = gHkEMaNnJNjTThODuUzp84475353;     gHkEMaNnJNjTThODuUzp84475353 = gHkEMaNnJNjTThODuUzp79400159;     gHkEMaNnJNjTThODuUzp79400159 = gHkEMaNnJNjTThODuUzp31075080;     gHkEMaNnJNjTThODuUzp31075080 = gHkEMaNnJNjTThODuUzp83881735;     gHkEMaNnJNjTThODuUzp83881735 = gHkEMaNnJNjTThODuUzp56610089;     gHkEMaNnJNjTThODuUzp56610089 = gHkEMaNnJNjTThODuUzp74667595;     gHkEMaNnJNjTThODuUzp74667595 = gHkEMaNnJNjTThODuUzp31018822;     gHkEMaNnJNjTThODuUzp31018822 = gHkEMaNnJNjTThODuUzp99977785;     gHkEMaNnJNjTThODuUzp99977785 = gHkEMaNnJNjTThODuUzp9023844;     gHkEMaNnJNjTThODuUzp9023844 = gHkEMaNnJNjTThODuUzp15146931;     gHkEMaNnJNjTThODuUzp15146931 = gHkEMaNnJNjTThODuUzp55085432;     gHkEMaNnJNjTThODuUzp55085432 = gHkEMaNnJNjTThODuUzp336478;     gHkEMaNnJNjTThODuUzp336478 = gHkEMaNnJNjTThODuUzp97035858;     gHkEMaNnJNjTThODuUzp97035858 = gHkEMaNnJNjTThODuUzp54741024;     gHkEMaNnJNjTThODuUzp54741024 = gHkEMaNnJNjTThODuUzp73460717;     gHkEMaNnJNjTThODuUzp73460717 = gHkEMaNnJNjTThODuUzp12765942;     gHkEMaNnJNjTThODuUzp12765942 = gHkEMaNnJNjTThODuUzp69208724;     gHkEMaNnJNjTThODuUzp69208724 = gHkEMaNnJNjTThODuUzp84336145;     gHkEMaNnJNjTThODuUzp84336145 = gHkEMaNnJNjTThODuUzp28347333;     gHkEMaNnJNjTThODuUzp28347333 = gHkEMaNnJNjTThODuUzp23579771;     gHkEMaNnJNjTThODuUzp23579771 = gHkEMaNnJNjTThODuUzp94031717;     gHkEMaNnJNjTThODuUzp94031717 = gHkEMaNnJNjTThODuUzp98928180;     gHkEMaNnJNjTThODuUzp98928180 = gHkEMaNnJNjTThODuUzp21272228;     gHkEMaNnJNjTThODuUzp21272228 = gHkEMaNnJNjTThODuUzp4190268;     gHkEMaNnJNjTThODuUzp4190268 = gHkEMaNnJNjTThODuUzp46034232;     gHkEMaNnJNjTThODuUzp46034232 = gHkEMaNnJNjTThODuUzp41898949;     gHkEMaNnJNjTThODuUzp41898949 = gHkEMaNnJNjTThODuUzp14623718;     gHkEMaNnJNjTThODuUzp14623718 = gHkEMaNnJNjTThODuUzp40707285;     gHkEMaNnJNjTThODuUzp40707285 = gHkEMaNnJNjTThODuUzp4995401;     gHkEMaNnJNjTThODuUzp4995401 = gHkEMaNnJNjTThODuUzp13398573;     gHkEMaNnJNjTThODuUzp13398573 = gHkEMaNnJNjTThODuUzp63003362;     gHkEMaNnJNjTThODuUzp63003362 = gHkEMaNnJNjTThODuUzp71673457;     gHkEMaNnJNjTThODuUzp71673457 = gHkEMaNnJNjTThODuUzp65811018;     gHkEMaNnJNjTThODuUzp65811018 = gHkEMaNnJNjTThODuUzp84966773;     gHkEMaNnJNjTThODuUzp84966773 = gHkEMaNnJNjTThODuUzp71162830;     gHkEMaNnJNjTThODuUzp71162830 = gHkEMaNnJNjTThODuUzp78500850;     gHkEMaNnJNjTThODuUzp78500850 = gHkEMaNnJNjTThODuUzp22050822;     gHkEMaNnJNjTThODuUzp22050822 = gHkEMaNnJNjTThODuUzp78781033;     gHkEMaNnJNjTThODuUzp78781033 = gHkEMaNnJNjTThODuUzp13735704;     gHkEMaNnJNjTThODuUzp13735704 = gHkEMaNnJNjTThODuUzp32748629;     gHkEMaNnJNjTThODuUzp32748629 = gHkEMaNnJNjTThODuUzp99235279;     gHkEMaNnJNjTThODuUzp99235279 = gHkEMaNnJNjTThODuUzp45082811;     gHkEMaNnJNjTThODuUzp45082811 = gHkEMaNnJNjTThODuUzp57077050;     gHkEMaNnJNjTThODuUzp57077050 = gHkEMaNnJNjTThODuUzp26091718;     gHkEMaNnJNjTThODuUzp26091718 = gHkEMaNnJNjTThODuUzp61856562;     gHkEMaNnJNjTThODuUzp61856562 = gHkEMaNnJNjTThODuUzp83526461;     gHkEMaNnJNjTThODuUzp83526461 = gHkEMaNnJNjTThODuUzp86160548;     gHkEMaNnJNjTThODuUzp86160548 = gHkEMaNnJNjTThODuUzp57752183;     gHkEMaNnJNjTThODuUzp57752183 = gHkEMaNnJNjTThODuUzp60220889;     gHkEMaNnJNjTThODuUzp60220889 = gHkEMaNnJNjTThODuUzp3705745;     gHkEMaNnJNjTThODuUzp3705745 = gHkEMaNnJNjTThODuUzp88772533;     gHkEMaNnJNjTThODuUzp88772533 = gHkEMaNnJNjTThODuUzp43745208;     gHkEMaNnJNjTThODuUzp43745208 = gHkEMaNnJNjTThODuUzp49036941;     gHkEMaNnJNjTThODuUzp49036941 = gHkEMaNnJNjTThODuUzp20584632;     gHkEMaNnJNjTThODuUzp20584632 = gHkEMaNnJNjTThODuUzp66499708;     gHkEMaNnJNjTThODuUzp66499708 = gHkEMaNnJNjTThODuUzp85547172;     gHkEMaNnJNjTThODuUzp85547172 = gHkEMaNnJNjTThODuUzp58127932;     gHkEMaNnJNjTThODuUzp58127932 = gHkEMaNnJNjTThODuUzp26884813;     gHkEMaNnJNjTThODuUzp26884813 = gHkEMaNnJNjTThODuUzp37847504;     gHkEMaNnJNjTThODuUzp37847504 = gHkEMaNnJNjTThODuUzp14711141;     gHkEMaNnJNjTThODuUzp14711141 = gHkEMaNnJNjTThODuUzp60043877;     gHkEMaNnJNjTThODuUzp60043877 = gHkEMaNnJNjTThODuUzp90311537;     gHkEMaNnJNjTThODuUzp90311537 = gHkEMaNnJNjTThODuUzp94982384;     gHkEMaNnJNjTThODuUzp94982384 = gHkEMaNnJNjTThODuUzp95625271;     gHkEMaNnJNjTThODuUzp95625271 = gHkEMaNnJNjTThODuUzp52143569;     gHkEMaNnJNjTThODuUzp52143569 = gHkEMaNnJNjTThODuUzp83411974;     gHkEMaNnJNjTThODuUzp83411974 = gHkEMaNnJNjTThODuUzp34525459;     gHkEMaNnJNjTThODuUzp34525459 = gHkEMaNnJNjTThODuUzp12069086;     gHkEMaNnJNjTThODuUzp12069086 = gHkEMaNnJNjTThODuUzp83578194;     gHkEMaNnJNjTThODuUzp83578194 = gHkEMaNnJNjTThODuUzp94959867;     gHkEMaNnJNjTThODuUzp94959867 = gHkEMaNnJNjTThODuUzp90715120;     gHkEMaNnJNjTThODuUzp90715120 = gHkEMaNnJNjTThODuUzp90427691;     gHkEMaNnJNjTThODuUzp90427691 = gHkEMaNnJNjTThODuUzp70600442;     gHkEMaNnJNjTThODuUzp70600442 = gHkEMaNnJNjTThODuUzp95598703;     gHkEMaNnJNjTThODuUzp95598703 = gHkEMaNnJNjTThODuUzp24344492;     gHkEMaNnJNjTThODuUzp24344492 = gHkEMaNnJNjTThODuUzp48948907;     gHkEMaNnJNjTThODuUzp48948907 = gHkEMaNnJNjTThODuUzp41851131;     gHkEMaNnJNjTThODuUzp41851131 = gHkEMaNnJNjTThODuUzp95180509;     gHkEMaNnJNjTThODuUzp95180509 = gHkEMaNnJNjTThODuUzp42333705;     gHkEMaNnJNjTThODuUzp42333705 = gHkEMaNnJNjTThODuUzp62507770;     gHkEMaNnJNjTThODuUzp62507770 = gHkEMaNnJNjTThODuUzp55738400;     gHkEMaNnJNjTThODuUzp55738400 = gHkEMaNnJNjTThODuUzp56871534;     gHkEMaNnJNjTThODuUzp56871534 = gHkEMaNnJNjTThODuUzp80486396;     gHkEMaNnJNjTThODuUzp80486396 = gHkEMaNnJNjTThODuUzp1289657;     gHkEMaNnJNjTThODuUzp1289657 = gHkEMaNnJNjTThODuUzp24626039;     gHkEMaNnJNjTThODuUzp24626039 = gHkEMaNnJNjTThODuUzp19258155;     gHkEMaNnJNjTThODuUzp19258155 = gHkEMaNnJNjTThODuUzp22636517;     gHkEMaNnJNjTThODuUzp22636517 = gHkEMaNnJNjTThODuUzp45226387;     gHkEMaNnJNjTThODuUzp45226387 = gHkEMaNnJNjTThODuUzp18467065;     gHkEMaNnJNjTThODuUzp18467065 = gHkEMaNnJNjTThODuUzp16941995;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gpunouMNwTAsLolhEqQe68832905() {     double rtvBAhociDEiMkOQDgeq42527271 = -583260870;    double rtvBAhociDEiMkOQDgeq80260082 = 68624312;    double rtvBAhociDEiMkOQDgeq16075583 = -855635439;    double rtvBAhociDEiMkOQDgeq58215137 = -557054068;    double rtvBAhociDEiMkOQDgeq3023611 = 42347902;    double rtvBAhociDEiMkOQDgeq33480963 = -355228739;    double rtvBAhociDEiMkOQDgeq63302102 = -384110044;    double rtvBAhociDEiMkOQDgeq43091478 = -829577192;    double rtvBAhociDEiMkOQDgeq2713566 = -820254483;    double rtvBAhociDEiMkOQDgeq63322784 = -521131886;    double rtvBAhociDEiMkOQDgeq74864979 = -447537419;    double rtvBAhociDEiMkOQDgeq19793010 = -904697875;    double rtvBAhociDEiMkOQDgeq55980938 = 50757732;    double rtvBAhociDEiMkOQDgeq22634797 = -158610416;    double rtvBAhociDEiMkOQDgeq40150624 = -170674714;    double rtvBAhociDEiMkOQDgeq64331132 = -193294613;    double rtvBAhociDEiMkOQDgeq74465711 = -706526532;    double rtvBAhociDEiMkOQDgeq52134726 = -711684940;    double rtvBAhociDEiMkOQDgeq61442507 = -95153253;    double rtvBAhociDEiMkOQDgeq70185310 = 84202163;    double rtvBAhociDEiMkOQDgeq16156196 = -168640969;    double rtvBAhociDEiMkOQDgeq56588538 = -939076468;    double rtvBAhociDEiMkOQDgeq4100275 = -351670455;    double rtvBAhociDEiMkOQDgeq98749900 = 71213206;    double rtvBAhociDEiMkOQDgeq9644164 = -838759075;    double rtvBAhociDEiMkOQDgeq2949854 = 84249609;    double rtvBAhociDEiMkOQDgeq95153850 = -530917365;    double rtvBAhociDEiMkOQDgeq33107525 = -21585681;    double rtvBAhociDEiMkOQDgeq92213358 = -205673837;    double rtvBAhociDEiMkOQDgeq81669823 = -127918002;    double rtvBAhociDEiMkOQDgeq45626405 = -416907473;    double rtvBAhociDEiMkOQDgeq31221651 = -605265797;    double rtvBAhociDEiMkOQDgeq20115233 = -465531352;    double rtvBAhociDEiMkOQDgeq74859298 = -26522751;    double rtvBAhociDEiMkOQDgeq5595256 = -623848006;    double rtvBAhociDEiMkOQDgeq50826082 = -697138269;    double rtvBAhociDEiMkOQDgeq91544333 = -429471719;    double rtvBAhociDEiMkOQDgeq90196404 = -304279046;    double rtvBAhociDEiMkOQDgeq81630863 = -671003648;    double rtvBAhociDEiMkOQDgeq13779075 = -118136842;    double rtvBAhociDEiMkOQDgeq60549853 = -305431949;    double rtvBAhociDEiMkOQDgeq7012952 = -421705218;    double rtvBAhociDEiMkOQDgeq20873677 = -144213506;    double rtvBAhociDEiMkOQDgeq36604298 = -134859629;    double rtvBAhociDEiMkOQDgeq76947649 = -236240835;    double rtvBAhociDEiMkOQDgeq37563152 = -273973097;    double rtvBAhociDEiMkOQDgeq78269304 = -134352560;    double rtvBAhociDEiMkOQDgeq30163592 = -283509847;    double rtvBAhociDEiMkOQDgeq29592354 = -932197116;    double rtvBAhociDEiMkOQDgeq67545698 = -976733798;    double rtvBAhociDEiMkOQDgeq42884876 = -959565437;    double rtvBAhociDEiMkOQDgeq26493859 = -670831906;    double rtvBAhociDEiMkOQDgeq63156415 = -806717471;    double rtvBAhociDEiMkOQDgeq25975959 = -776215061;    double rtvBAhociDEiMkOQDgeq52676709 = -365011514;    double rtvBAhociDEiMkOQDgeq85938732 = -644184403;    double rtvBAhociDEiMkOQDgeq76159807 = -579705233;    double rtvBAhociDEiMkOQDgeq17325682 = -826848645;    double rtvBAhociDEiMkOQDgeq48570974 = -718294994;    double rtvBAhociDEiMkOQDgeq73758 = 58098292;    double rtvBAhociDEiMkOQDgeq38327112 = -824311374;    double rtvBAhociDEiMkOQDgeq30194578 = -262524363;    double rtvBAhociDEiMkOQDgeq50878120 = -523903355;    double rtvBAhociDEiMkOQDgeq21043743 = -592336481;    double rtvBAhociDEiMkOQDgeq17696380 = -4224414;    double rtvBAhociDEiMkOQDgeq43643329 = -842271622;    double rtvBAhociDEiMkOQDgeq99677776 = -339166524;    double rtvBAhociDEiMkOQDgeq81121639 = -922719517;    double rtvBAhociDEiMkOQDgeq17039542 = -534762410;    double rtvBAhociDEiMkOQDgeq89324541 = -473536446;    double rtvBAhociDEiMkOQDgeq72786798 = -763822895;    double rtvBAhociDEiMkOQDgeq84269306 = -302247487;    double rtvBAhociDEiMkOQDgeq70503862 = 59318708;    double rtvBAhociDEiMkOQDgeq47663432 = -977016412;    double rtvBAhociDEiMkOQDgeq9635458 = -610365889;    double rtvBAhociDEiMkOQDgeq9143244 = -746935752;    double rtvBAhociDEiMkOQDgeq35714862 = -694862962;    double rtvBAhociDEiMkOQDgeq67495977 = -116810827;    double rtvBAhociDEiMkOQDgeq21802252 = -692545959;    double rtvBAhociDEiMkOQDgeq72081011 = -464785979;    double rtvBAhociDEiMkOQDgeq24680550 = -781397831;    double rtvBAhociDEiMkOQDgeq64990258 = -147407519;    double rtvBAhociDEiMkOQDgeq3515172 = -89388566;    double rtvBAhociDEiMkOQDgeq24667660 = -228940040;    double rtvBAhociDEiMkOQDgeq38784947 = -168352565;    double rtvBAhociDEiMkOQDgeq19132546 = -746075568;    double rtvBAhociDEiMkOQDgeq68065235 = -798548327;    double rtvBAhociDEiMkOQDgeq94139273 = -689316292;    double rtvBAhociDEiMkOQDgeq22182590 = -661511238;    double rtvBAhociDEiMkOQDgeq19656523 = -979663603;    double rtvBAhociDEiMkOQDgeq74666275 = -17433037;    double rtvBAhociDEiMkOQDgeq74218652 = -602623074;    double rtvBAhociDEiMkOQDgeq41625431 = -585984053;    double rtvBAhociDEiMkOQDgeq81557106 = -629101941;    double rtvBAhociDEiMkOQDgeq75451962 = -293825468;    double rtvBAhociDEiMkOQDgeq30355276 = 57092414;    double rtvBAhociDEiMkOQDgeq56134832 = -897801863;    double rtvBAhociDEiMkOQDgeq99829934 = -551877025;    double rtvBAhociDEiMkOQDgeq18907919 = -30635216;    double rtvBAhociDEiMkOQDgeq33304320 = -583260870;     rtvBAhociDEiMkOQDgeq42527271 = rtvBAhociDEiMkOQDgeq80260082;     rtvBAhociDEiMkOQDgeq80260082 = rtvBAhociDEiMkOQDgeq16075583;     rtvBAhociDEiMkOQDgeq16075583 = rtvBAhociDEiMkOQDgeq58215137;     rtvBAhociDEiMkOQDgeq58215137 = rtvBAhociDEiMkOQDgeq3023611;     rtvBAhociDEiMkOQDgeq3023611 = rtvBAhociDEiMkOQDgeq33480963;     rtvBAhociDEiMkOQDgeq33480963 = rtvBAhociDEiMkOQDgeq63302102;     rtvBAhociDEiMkOQDgeq63302102 = rtvBAhociDEiMkOQDgeq43091478;     rtvBAhociDEiMkOQDgeq43091478 = rtvBAhociDEiMkOQDgeq2713566;     rtvBAhociDEiMkOQDgeq2713566 = rtvBAhociDEiMkOQDgeq63322784;     rtvBAhociDEiMkOQDgeq63322784 = rtvBAhociDEiMkOQDgeq74864979;     rtvBAhociDEiMkOQDgeq74864979 = rtvBAhociDEiMkOQDgeq19793010;     rtvBAhociDEiMkOQDgeq19793010 = rtvBAhociDEiMkOQDgeq55980938;     rtvBAhociDEiMkOQDgeq55980938 = rtvBAhociDEiMkOQDgeq22634797;     rtvBAhociDEiMkOQDgeq22634797 = rtvBAhociDEiMkOQDgeq40150624;     rtvBAhociDEiMkOQDgeq40150624 = rtvBAhociDEiMkOQDgeq64331132;     rtvBAhociDEiMkOQDgeq64331132 = rtvBAhociDEiMkOQDgeq74465711;     rtvBAhociDEiMkOQDgeq74465711 = rtvBAhociDEiMkOQDgeq52134726;     rtvBAhociDEiMkOQDgeq52134726 = rtvBAhociDEiMkOQDgeq61442507;     rtvBAhociDEiMkOQDgeq61442507 = rtvBAhociDEiMkOQDgeq70185310;     rtvBAhociDEiMkOQDgeq70185310 = rtvBAhociDEiMkOQDgeq16156196;     rtvBAhociDEiMkOQDgeq16156196 = rtvBAhociDEiMkOQDgeq56588538;     rtvBAhociDEiMkOQDgeq56588538 = rtvBAhociDEiMkOQDgeq4100275;     rtvBAhociDEiMkOQDgeq4100275 = rtvBAhociDEiMkOQDgeq98749900;     rtvBAhociDEiMkOQDgeq98749900 = rtvBAhociDEiMkOQDgeq9644164;     rtvBAhociDEiMkOQDgeq9644164 = rtvBAhociDEiMkOQDgeq2949854;     rtvBAhociDEiMkOQDgeq2949854 = rtvBAhociDEiMkOQDgeq95153850;     rtvBAhociDEiMkOQDgeq95153850 = rtvBAhociDEiMkOQDgeq33107525;     rtvBAhociDEiMkOQDgeq33107525 = rtvBAhociDEiMkOQDgeq92213358;     rtvBAhociDEiMkOQDgeq92213358 = rtvBAhociDEiMkOQDgeq81669823;     rtvBAhociDEiMkOQDgeq81669823 = rtvBAhociDEiMkOQDgeq45626405;     rtvBAhociDEiMkOQDgeq45626405 = rtvBAhociDEiMkOQDgeq31221651;     rtvBAhociDEiMkOQDgeq31221651 = rtvBAhociDEiMkOQDgeq20115233;     rtvBAhociDEiMkOQDgeq20115233 = rtvBAhociDEiMkOQDgeq74859298;     rtvBAhociDEiMkOQDgeq74859298 = rtvBAhociDEiMkOQDgeq5595256;     rtvBAhociDEiMkOQDgeq5595256 = rtvBAhociDEiMkOQDgeq50826082;     rtvBAhociDEiMkOQDgeq50826082 = rtvBAhociDEiMkOQDgeq91544333;     rtvBAhociDEiMkOQDgeq91544333 = rtvBAhociDEiMkOQDgeq90196404;     rtvBAhociDEiMkOQDgeq90196404 = rtvBAhociDEiMkOQDgeq81630863;     rtvBAhociDEiMkOQDgeq81630863 = rtvBAhociDEiMkOQDgeq13779075;     rtvBAhociDEiMkOQDgeq13779075 = rtvBAhociDEiMkOQDgeq60549853;     rtvBAhociDEiMkOQDgeq60549853 = rtvBAhociDEiMkOQDgeq7012952;     rtvBAhociDEiMkOQDgeq7012952 = rtvBAhociDEiMkOQDgeq20873677;     rtvBAhociDEiMkOQDgeq20873677 = rtvBAhociDEiMkOQDgeq36604298;     rtvBAhociDEiMkOQDgeq36604298 = rtvBAhociDEiMkOQDgeq76947649;     rtvBAhociDEiMkOQDgeq76947649 = rtvBAhociDEiMkOQDgeq37563152;     rtvBAhociDEiMkOQDgeq37563152 = rtvBAhociDEiMkOQDgeq78269304;     rtvBAhociDEiMkOQDgeq78269304 = rtvBAhociDEiMkOQDgeq30163592;     rtvBAhociDEiMkOQDgeq30163592 = rtvBAhociDEiMkOQDgeq29592354;     rtvBAhociDEiMkOQDgeq29592354 = rtvBAhociDEiMkOQDgeq67545698;     rtvBAhociDEiMkOQDgeq67545698 = rtvBAhociDEiMkOQDgeq42884876;     rtvBAhociDEiMkOQDgeq42884876 = rtvBAhociDEiMkOQDgeq26493859;     rtvBAhociDEiMkOQDgeq26493859 = rtvBAhociDEiMkOQDgeq63156415;     rtvBAhociDEiMkOQDgeq63156415 = rtvBAhociDEiMkOQDgeq25975959;     rtvBAhociDEiMkOQDgeq25975959 = rtvBAhociDEiMkOQDgeq52676709;     rtvBAhociDEiMkOQDgeq52676709 = rtvBAhociDEiMkOQDgeq85938732;     rtvBAhociDEiMkOQDgeq85938732 = rtvBAhociDEiMkOQDgeq76159807;     rtvBAhociDEiMkOQDgeq76159807 = rtvBAhociDEiMkOQDgeq17325682;     rtvBAhociDEiMkOQDgeq17325682 = rtvBAhociDEiMkOQDgeq48570974;     rtvBAhociDEiMkOQDgeq48570974 = rtvBAhociDEiMkOQDgeq73758;     rtvBAhociDEiMkOQDgeq73758 = rtvBAhociDEiMkOQDgeq38327112;     rtvBAhociDEiMkOQDgeq38327112 = rtvBAhociDEiMkOQDgeq30194578;     rtvBAhociDEiMkOQDgeq30194578 = rtvBAhociDEiMkOQDgeq50878120;     rtvBAhociDEiMkOQDgeq50878120 = rtvBAhociDEiMkOQDgeq21043743;     rtvBAhociDEiMkOQDgeq21043743 = rtvBAhociDEiMkOQDgeq17696380;     rtvBAhociDEiMkOQDgeq17696380 = rtvBAhociDEiMkOQDgeq43643329;     rtvBAhociDEiMkOQDgeq43643329 = rtvBAhociDEiMkOQDgeq99677776;     rtvBAhociDEiMkOQDgeq99677776 = rtvBAhociDEiMkOQDgeq81121639;     rtvBAhociDEiMkOQDgeq81121639 = rtvBAhociDEiMkOQDgeq17039542;     rtvBAhociDEiMkOQDgeq17039542 = rtvBAhociDEiMkOQDgeq89324541;     rtvBAhociDEiMkOQDgeq89324541 = rtvBAhociDEiMkOQDgeq72786798;     rtvBAhociDEiMkOQDgeq72786798 = rtvBAhociDEiMkOQDgeq84269306;     rtvBAhociDEiMkOQDgeq84269306 = rtvBAhociDEiMkOQDgeq70503862;     rtvBAhociDEiMkOQDgeq70503862 = rtvBAhociDEiMkOQDgeq47663432;     rtvBAhociDEiMkOQDgeq47663432 = rtvBAhociDEiMkOQDgeq9635458;     rtvBAhociDEiMkOQDgeq9635458 = rtvBAhociDEiMkOQDgeq9143244;     rtvBAhociDEiMkOQDgeq9143244 = rtvBAhociDEiMkOQDgeq35714862;     rtvBAhociDEiMkOQDgeq35714862 = rtvBAhociDEiMkOQDgeq67495977;     rtvBAhociDEiMkOQDgeq67495977 = rtvBAhociDEiMkOQDgeq21802252;     rtvBAhociDEiMkOQDgeq21802252 = rtvBAhociDEiMkOQDgeq72081011;     rtvBAhociDEiMkOQDgeq72081011 = rtvBAhociDEiMkOQDgeq24680550;     rtvBAhociDEiMkOQDgeq24680550 = rtvBAhociDEiMkOQDgeq64990258;     rtvBAhociDEiMkOQDgeq64990258 = rtvBAhociDEiMkOQDgeq3515172;     rtvBAhociDEiMkOQDgeq3515172 = rtvBAhociDEiMkOQDgeq24667660;     rtvBAhociDEiMkOQDgeq24667660 = rtvBAhociDEiMkOQDgeq38784947;     rtvBAhociDEiMkOQDgeq38784947 = rtvBAhociDEiMkOQDgeq19132546;     rtvBAhociDEiMkOQDgeq19132546 = rtvBAhociDEiMkOQDgeq68065235;     rtvBAhociDEiMkOQDgeq68065235 = rtvBAhociDEiMkOQDgeq94139273;     rtvBAhociDEiMkOQDgeq94139273 = rtvBAhociDEiMkOQDgeq22182590;     rtvBAhociDEiMkOQDgeq22182590 = rtvBAhociDEiMkOQDgeq19656523;     rtvBAhociDEiMkOQDgeq19656523 = rtvBAhociDEiMkOQDgeq74666275;     rtvBAhociDEiMkOQDgeq74666275 = rtvBAhociDEiMkOQDgeq74218652;     rtvBAhociDEiMkOQDgeq74218652 = rtvBAhociDEiMkOQDgeq41625431;     rtvBAhociDEiMkOQDgeq41625431 = rtvBAhociDEiMkOQDgeq81557106;     rtvBAhociDEiMkOQDgeq81557106 = rtvBAhociDEiMkOQDgeq75451962;     rtvBAhociDEiMkOQDgeq75451962 = rtvBAhociDEiMkOQDgeq30355276;     rtvBAhociDEiMkOQDgeq30355276 = rtvBAhociDEiMkOQDgeq56134832;     rtvBAhociDEiMkOQDgeq56134832 = rtvBAhociDEiMkOQDgeq99829934;     rtvBAhociDEiMkOQDgeq99829934 = rtvBAhociDEiMkOQDgeq18907919;     rtvBAhociDEiMkOQDgeq18907919 = rtvBAhociDEiMkOQDgeq33304320;     rtvBAhociDEiMkOQDgeq33304320 = rtvBAhociDEiMkOQDgeq42527271;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BCWJZfwzMPNxbSohOlhT96427161() {     double sPsvwJiDsNxGEfCbexfY68112548 = -8293971;    double sPsvwJiDsNxGEfCbexfY76657225 = -186853464;    double sPsvwJiDsNxGEfCbexfY48954758 = -658096975;    double sPsvwJiDsNxGEfCbexfY3937067 = -564376962;    double sPsvwJiDsNxGEfCbexfY72365615 = -4574385;    double sPsvwJiDsNxGEfCbexfY50490239 = -510209742;    double sPsvwJiDsNxGEfCbexfY68622947 = -966381251;    double sPsvwJiDsNxGEfCbexfY58101602 = -110259071;    double sPsvwJiDsNxGEfCbexfY28042859 = -216478527;    double sPsvwJiDsNxGEfCbexfY82481166 = -855693410;    double sPsvwJiDsNxGEfCbexfY89198533 = -757965306;    double sPsvwJiDsNxGEfCbexfY55110665 = -248406782;    double sPsvwJiDsNxGEfCbexfY32561716 = -813490633;    double sPsvwJiDsNxGEfCbexfY14194514 = -133134709;    double sPsvwJiDsNxGEfCbexfY96419511 = -59220672;    double sPsvwJiDsNxGEfCbexfY72052175 = -629486090;    double sPsvwJiDsNxGEfCbexfY74263827 = -394846327;    double sPsvwJiDsNxGEfCbexfY73250631 = -127962300;    double sPsvwJiDsNxGEfCbexfY22907228 = -878127111;    double sPsvwJiDsNxGEfCbexfY31346777 = -990237213;    double sPsvwJiDsNxGEfCbexfY17165462 = -630340750;    double sPsvwJiDsNxGEfCbexfY58091645 = -258809397;    double sPsvwJiDsNxGEfCbexfY7864073 = -395762561;    double sPsvwJiDsNxGEfCbexfY463944 = -887635773;    double sPsvwJiDsNxGEfCbexfY64547303 = -406255217;    double sPsvwJiDsNxGEfCbexfY32438990 = -580200932;    double sPsvwJiDsNxGEfCbexfY77541760 = -96730500;    double sPsvwJiDsNxGEfCbexfY97006324 = -251247914;    double sPsvwJiDsNxGEfCbexfY90571 = -441967688;    double sPsvwJiDsNxGEfCbexfY34992314 = -36827564;    double sPsvwJiDsNxGEfCbexfY67673039 = -195554504;    double sPsvwJiDsNxGEfCbexfY68411583 = -879061631;    double sPsvwJiDsNxGEfCbexfY41302285 = -913352489;    double sPsvwJiDsNxGEfCbexfY28446370 = -82667723;    double sPsvwJiDsNxGEfCbexfY7000244 = -225283918;    double sPsvwJiDsNxGEfCbexfY55617933 = -114732985;    double sPsvwJiDsNxGEfCbexfY41189719 = -909983663;    double sPsvwJiDsNxGEfCbexfY65769091 = -152993871;    double sPsvwJiDsNxGEfCbexfY22554443 = -29341970;    double sPsvwJiDsNxGEfCbexfY22562749 = -588107161;    double sPsvwJiDsNxGEfCbexfY7701135 = -880492139;    double sPsvwJiDsNxGEfCbexfY51022542 = -930743722;    double sPsvwJiDsNxGEfCbexfY70073896 = -172819887;    double sPsvwJiDsNxGEfCbexfY7397577 = -603275228;    double sPsvwJiDsNxGEfCbexfY68928525 = -175261953;    double sPsvwJiDsNxGEfCbexfY3963475 = -796520634;    double sPsvwJiDsNxGEfCbexfY78037758 = -166821421;    double sPsvwJiDsNxGEfCbexfY38276363 = -476692693;    double sPsvwJiDsNxGEfCbexfY80403673 = 68126657;    double sPsvwJiDsNxGEfCbexfY21355694 = -824756951;    double sPsvwJiDsNxGEfCbexfY53021123 = -108224040;    double sPsvwJiDsNxGEfCbexfY53752438 = -848911085;    double sPsvwJiDsNxGEfCbexfY81230018 = -274616020;    double sPsvwJiDsNxGEfCbexfY94874867 = 8721157;    double sPsvwJiDsNxGEfCbexfY79261700 = -840101931;    double sPsvwJiDsNxGEfCbexfY10020903 = -749484574;    double sPsvwJiDsNxGEfCbexfY68793152 = -791090904;    double sPsvwJiDsNxGEfCbexfY48490815 = -770461202;    double sPsvwJiDsNxGEfCbexfY39389764 = -58121745;    double sPsvwJiDsNxGEfCbexfY39926626 = -424373453;    double sPsvwJiDsNxGEfCbexfY72948479 = -313479243;    double sPsvwJiDsNxGEfCbexfY71616622 = -615133338;    double sPsvwJiDsNxGEfCbexfY58011031 = -668291384;    double sPsvwJiDsNxGEfCbexfY93050544 = -79650964;    double sPsvwJiDsNxGEfCbexfY14808128 = -560138906;    double sPsvwJiDsNxGEfCbexfY20786951 = -878903675;    double sPsvwJiDsNxGEfCbexfY13808381 = -335054293;    double sPsvwJiDsNxGEfCbexfY4115346 = -630822911;    double sPsvwJiDsNxGEfCbexfY7194270 = -907850792;    double sPsvwJiDsNxGEfCbexfY40801579 = -944487688;    double sPsvwJiDsNxGEfCbexfY30862456 = -719502427;    double sPsvwJiDsNxGEfCbexfY8494737 = -141852456;    double sPsvwJiDsNxGEfCbexfY50696188 = 1379669;    double sPsvwJiDsNxGEfCbexfY344480 = -190019950;    double sPsvwJiDsNxGEfCbexfY23645643 = -9745074;    double sPsvwJiDsNxGEfCbexfY66142919 = -699597029;    double sPsvwJiDsNxGEfCbexfY88017749 = 14010490;    double sPsvwJiDsNxGEfCbexfY466497 = -792487333;    double sPsvwJiDsNxGEfCbexfY31535418 = -612373821;    double sPsvwJiDsNxGEfCbexfY60583828 = -609734584;    double sPsvwJiDsNxGEfCbexfY54401232 = -313379512;    double sPsvwJiDsNxGEfCbexfY39265397 = -620037807;    double sPsvwJiDsNxGEfCbexfY16602652 = -219374572;    double sPsvwJiDsNxGEfCbexfY78734877 = -617210737;    double sPsvwJiDsNxGEfCbexfY81971190 = -928603525;    double sPsvwJiDsNxGEfCbexfY13920601 = -346643420;    double sPsvwJiDsNxGEfCbexfY87181564 = -504445612;    double sPsvwJiDsNxGEfCbexfY46427417 = -822073647;    double sPsvwJiDsNxGEfCbexfY49184669 = -242565792;    double sPsvwJiDsNxGEfCbexfY96979340 = -475799344;    double sPsvwJiDsNxGEfCbexfY86824780 = -323642081;    double sPsvwJiDsNxGEfCbexfY92698904 = -39522462;    double sPsvwJiDsNxGEfCbexfY26379327 = 5127873;    double sPsvwJiDsNxGEfCbexfY82627817 = -604968517;    double sPsvwJiDsNxGEfCbexfY49614269 = -174627919;    double sPsvwJiDsNxGEfCbexfY36084512 = -165358802;    double sPsvwJiDsNxGEfCbexfY93011510 = -162452339;    double sPsvwJiDsNxGEfCbexfY77023352 = 6831076;    double sPsvwJiDsNxGEfCbexfY92589449 = 56863678;    double sPsvwJiDsNxGEfCbexfY48141575 = -8293971;     sPsvwJiDsNxGEfCbexfY68112548 = sPsvwJiDsNxGEfCbexfY76657225;     sPsvwJiDsNxGEfCbexfY76657225 = sPsvwJiDsNxGEfCbexfY48954758;     sPsvwJiDsNxGEfCbexfY48954758 = sPsvwJiDsNxGEfCbexfY3937067;     sPsvwJiDsNxGEfCbexfY3937067 = sPsvwJiDsNxGEfCbexfY72365615;     sPsvwJiDsNxGEfCbexfY72365615 = sPsvwJiDsNxGEfCbexfY50490239;     sPsvwJiDsNxGEfCbexfY50490239 = sPsvwJiDsNxGEfCbexfY68622947;     sPsvwJiDsNxGEfCbexfY68622947 = sPsvwJiDsNxGEfCbexfY58101602;     sPsvwJiDsNxGEfCbexfY58101602 = sPsvwJiDsNxGEfCbexfY28042859;     sPsvwJiDsNxGEfCbexfY28042859 = sPsvwJiDsNxGEfCbexfY82481166;     sPsvwJiDsNxGEfCbexfY82481166 = sPsvwJiDsNxGEfCbexfY89198533;     sPsvwJiDsNxGEfCbexfY89198533 = sPsvwJiDsNxGEfCbexfY55110665;     sPsvwJiDsNxGEfCbexfY55110665 = sPsvwJiDsNxGEfCbexfY32561716;     sPsvwJiDsNxGEfCbexfY32561716 = sPsvwJiDsNxGEfCbexfY14194514;     sPsvwJiDsNxGEfCbexfY14194514 = sPsvwJiDsNxGEfCbexfY96419511;     sPsvwJiDsNxGEfCbexfY96419511 = sPsvwJiDsNxGEfCbexfY72052175;     sPsvwJiDsNxGEfCbexfY72052175 = sPsvwJiDsNxGEfCbexfY74263827;     sPsvwJiDsNxGEfCbexfY74263827 = sPsvwJiDsNxGEfCbexfY73250631;     sPsvwJiDsNxGEfCbexfY73250631 = sPsvwJiDsNxGEfCbexfY22907228;     sPsvwJiDsNxGEfCbexfY22907228 = sPsvwJiDsNxGEfCbexfY31346777;     sPsvwJiDsNxGEfCbexfY31346777 = sPsvwJiDsNxGEfCbexfY17165462;     sPsvwJiDsNxGEfCbexfY17165462 = sPsvwJiDsNxGEfCbexfY58091645;     sPsvwJiDsNxGEfCbexfY58091645 = sPsvwJiDsNxGEfCbexfY7864073;     sPsvwJiDsNxGEfCbexfY7864073 = sPsvwJiDsNxGEfCbexfY463944;     sPsvwJiDsNxGEfCbexfY463944 = sPsvwJiDsNxGEfCbexfY64547303;     sPsvwJiDsNxGEfCbexfY64547303 = sPsvwJiDsNxGEfCbexfY32438990;     sPsvwJiDsNxGEfCbexfY32438990 = sPsvwJiDsNxGEfCbexfY77541760;     sPsvwJiDsNxGEfCbexfY77541760 = sPsvwJiDsNxGEfCbexfY97006324;     sPsvwJiDsNxGEfCbexfY97006324 = sPsvwJiDsNxGEfCbexfY90571;     sPsvwJiDsNxGEfCbexfY90571 = sPsvwJiDsNxGEfCbexfY34992314;     sPsvwJiDsNxGEfCbexfY34992314 = sPsvwJiDsNxGEfCbexfY67673039;     sPsvwJiDsNxGEfCbexfY67673039 = sPsvwJiDsNxGEfCbexfY68411583;     sPsvwJiDsNxGEfCbexfY68411583 = sPsvwJiDsNxGEfCbexfY41302285;     sPsvwJiDsNxGEfCbexfY41302285 = sPsvwJiDsNxGEfCbexfY28446370;     sPsvwJiDsNxGEfCbexfY28446370 = sPsvwJiDsNxGEfCbexfY7000244;     sPsvwJiDsNxGEfCbexfY7000244 = sPsvwJiDsNxGEfCbexfY55617933;     sPsvwJiDsNxGEfCbexfY55617933 = sPsvwJiDsNxGEfCbexfY41189719;     sPsvwJiDsNxGEfCbexfY41189719 = sPsvwJiDsNxGEfCbexfY65769091;     sPsvwJiDsNxGEfCbexfY65769091 = sPsvwJiDsNxGEfCbexfY22554443;     sPsvwJiDsNxGEfCbexfY22554443 = sPsvwJiDsNxGEfCbexfY22562749;     sPsvwJiDsNxGEfCbexfY22562749 = sPsvwJiDsNxGEfCbexfY7701135;     sPsvwJiDsNxGEfCbexfY7701135 = sPsvwJiDsNxGEfCbexfY51022542;     sPsvwJiDsNxGEfCbexfY51022542 = sPsvwJiDsNxGEfCbexfY70073896;     sPsvwJiDsNxGEfCbexfY70073896 = sPsvwJiDsNxGEfCbexfY7397577;     sPsvwJiDsNxGEfCbexfY7397577 = sPsvwJiDsNxGEfCbexfY68928525;     sPsvwJiDsNxGEfCbexfY68928525 = sPsvwJiDsNxGEfCbexfY3963475;     sPsvwJiDsNxGEfCbexfY3963475 = sPsvwJiDsNxGEfCbexfY78037758;     sPsvwJiDsNxGEfCbexfY78037758 = sPsvwJiDsNxGEfCbexfY38276363;     sPsvwJiDsNxGEfCbexfY38276363 = sPsvwJiDsNxGEfCbexfY80403673;     sPsvwJiDsNxGEfCbexfY80403673 = sPsvwJiDsNxGEfCbexfY21355694;     sPsvwJiDsNxGEfCbexfY21355694 = sPsvwJiDsNxGEfCbexfY53021123;     sPsvwJiDsNxGEfCbexfY53021123 = sPsvwJiDsNxGEfCbexfY53752438;     sPsvwJiDsNxGEfCbexfY53752438 = sPsvwJiDsNxGEfCbexfY81230018;     sPsvwJiDsNxGEfCbexfY81230018 = sPsvwJiDsNxGEfCbexfY94874867;     sPsvwJiDsNxGEfCbexfY94874867 = sPsvwJiDsNxGEfCbexfY79261700;     sPsvwJiDsNxGEfCbexfY79261700 = sPsvwJiDsNxGEfCbexfY10020903;     sPsvwJiDsNxGEfCbexfY10020903 = sPsvwJiDsNxGEfCbexfY68793152;     sPsvwJiDsNxGEfCbexfY68793152 = sPsvwJiDsNxGEfCbexfY48490815;     sPsvwJiDsNxGEfCbexfY48490815 = sPsvwJiDsNxGEfCbexfY39389764;     sPsvwJiDsNxGEfCbexfY39389764 = sPsvwJiDsNxGEfCbexfY39926626;     sPsvwJiDsNxGEfCbexfY39926626 = sPsvwJiDsNxGEfCbexfY72948479;     sPsvwJiDsNxGEfCbexfY72948479 = sPsvwJiDsNxGEfCbexfY71616622;     sPsvwJiDsNxGEfCbexfY71616622 = sPsvwJiDsNxGEfCbexfY58011031;     sPsvwJiDsNxGEfCbexfY58011031 = sPsvwJiDsNxGEfCbexfY93050544;     sPsvwJiDsNxGEfCbexfY93050544 = sPsvwJiDsNxGEfCbexfY14808128;     sPsvwJiDsNxGEfCbexfY14808128 = sPsvwJiDsNxGEfCbexfY20786951;     sPsvwJiDsNxGEfCbexfY20786951 = sPsvwJiDsNxGEfCbexfY13808381;     sPsvwJiDsNxGEfCbexfY13808381 = sPsvwJiDsNxGEfCbexfY4115346;     sPsvwJiDsNxGEfCbexfY4115346 = sPsvwJiDsNxGEfCbexfY7194270;     sPsvwJiDsNxGEfCbexfY7194270 = sPsvwJiDsNxGEfCbexfY40801579;     sPsvwJiDsNxGEfCbexfY40801579 = sPsvwJiDsNxGEfCbexfY30862456;     sPsvwJiDsNxGEfCbexfY30862456 = sPsvwJiDsNxGEfCbexfY8494737;     sPsvwJiDsNxGEfCbexfY8494737 = sPsvwJiDsNxGEfCbexfY50696188;     sPsvwJiDsNxGEfCbexfY50696188 = sPsvwJiDsNxGEfCbexfY344480;     sPsvwJiDsNxGEfCbexfY344480 = sPsvwJiDsNxGEfCbexfY23645643;     sPsvwJiDsNxGEfCbexfY23645643 = sPsvwJiDsNxGEfCbexfY66142919;     sPsvwJiDsNxGEfCbexfY66142919 = sPsvwJiDsNxGEfCbexfY88017749;     sPsvwJiDsNxGEfCbexfY88017749 = sPsvwJiDsNxGEfCbexfY466497;     sPsvwJiDsNxGEfCbexfY466497 = sPsvwJiDsNxGEfCbexfY31535418;     sPsvwJiDsNxGEfCbexfY31535418 = sPsvwJiDsNxGEfCbexfY60583828;     sPsvwJiDsNxGEfCbexfY60583828 = sPsvwJiDsNxGEfCbexfY54401232;     sPsvwJiDsNxGEfCbexfY54401232 = sPsvwJiDsNxGEfCbexfY39265397;     sPsvwJiDsNxGEfCbexfY39265397 = sPsvwJiDsNxGEfCbexfY16602652;     sPsvwJiDsNxGEfCbexfY16602652 = sPsvwJiDsNxGEfCbexfY78734877;     sPsvwJiDsNxGEfCbexfY78734877 = sPsvwJiDsNxGEfCbexfY81971190;     sPsvwJiDsNxGEfCbexfY81971190 = sPsvwJiDsNxGEfCbexfY13920601;     sPsvwJiDsNxGEfCbexfY13920601 = sPsvwJiDsNxGEfCbexfY87181564;     sPsvwJiDsNxGEfCbexfY87181564 = sPsvwJiDsNxGEfCbexfY46427417;     sPsvwJiDsNxGEfCbexfY46427417 = sPsvwJiDsNxGEfCbexfY49184669;     sPsvwJiDsNxGEfCbexfY49184669 = sPsvwJiDsNxGEfCbexfY96979340;     sPsvwJiDsNxGEfCbexfY96979340 = sPsvwJiDsNxGEfCbexfY86824780;     sPsvwJiDsNxGEfCbexfY86824780 = sPsvwJiDsNxGEfCbexfY92698904;     sPsvwJiDsNxGEfCbexfY92698904 = sPsvwJiDsNxGEfCbexfY26379327;     sPsvwJiDsNxGEfCbexfY26379327 = sPsvwJiDsNxGEfCbexfY82627817;     sPsvwJiDsNxGEfCbexfY82627817 = sPsvwJiDsNxGEfCbexfY49614269;     sPsvwJiDsNxGEfCbexfY49614269 = sPsvwJiDsNxGEfCbexfY36084512;     sPsvwJiDsNxGEfCbexfY36084512 = sPsvwJiDsNxGEfCbexfY93011510;     sPsvwJiDsNxGEfCbexfY93011510 = sPsvwJiDsNxGEfCbexfY77023352;     sPsvwJiDsNxGEfCbexfY77023352 = sPsvwJiDsNxGEfCbexfY92589449;     sPsvwJiDsNxGEfCbexfY92589449 = sPsvwJiDsNxGEfCbexfY48141575;     sPsvwJiDsNxGEfCbexfY48141575 = sPsvwJiDsNxGEfCbexfY68112548;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MEDxuWQCKXzQWdqxkuPx24021418() {     double GomhVcDgNhVPJzEdsszk93697825 = -533327072;    double GomhVcDgNhVPJzEdsszk73054368 = -442331241;    double GomhVcDgNhVPJzEdsszk81833933 = -460558511;    double GomhVcDgNhVPJzEdsszk49658996 = -571699855;    double GomhVcDgNhVPJzEdsszk41707620 = -51496672;    double GomhVcDgNhVPJzEdsszk67499516 = -665190745;    double GomhVcDgNhVPJzEdsszk73943791 = -448652458;    double GomhVcDgNhVPJzEdsszk73111726 = -490940950;    double GomhVcDgNhVPJzEdsszk53372151 = -712702572;    double GomhVcDgNhVPJzEdsszk1639549 = -90254933;    double GomhVcDgNhVPJzEdsszk3532088 = 31606807;    double GomhVcDgNhVPJzEdsszk90428321 = -692115689;    double GomhVcDgNhVPJzEdsszk9142494 = -577738999;    double GomhVcDgNhVPJzEdsszk5754231 = -107659003;    double GomhVcDgNhVPJzEdsszk52688400 = 52233370;    double GomhVcDgNhVPJzEdsszk79773218 = 34322433;    double GomhVcDgNhVPJzEdsszk74061943 = -83166121;    double GomhVcDgNhVPJzEdsszk94366535 = -644239661;    double GomhVcDgNhVPJzEdsszk84371948 = -561100968;    double GomhVcDgNhVPJzEdsszk92508243 = -964676588;    double GomhVcDgNhVPJzEdsszk18174727 = 7959469;    double GomhVcDgNhVPJzEdsszk59594752 = -678542327;    double GomhVcDgNhVPJzEdsszk11627871 = -439854666;    double GomhVcDgNhVPJzEdsszk2177986 = -746484753;    double GomhVcDgNhVPJzEdsszk19450442 = 26248640;    double GomhVcDgNhVPJzEdsszk61928127 = -144651474;    double GomhVcDgNhVPJzEdsszk59929669 = -762543635;    double GomhVcDgNhVPJzEdsszk60905125 = -480910146;    double GomhVcDgNhVPJzEdsszk7967783 = -678261538;    double GomhVcDgNhVPJzEdsszk88314805 = 54262874;    double GomhVcDgNhVPJzEdsszk89719673 = 25798465;    double GomhVcDgNhVPJzEdsszk5601516 = -52857465;    double GomhVcDgNhVPJzEdsszk62489336 = -261173626;    double GomhVcDgNhVPJzEdsszk82033441 = -138812694;    double GomhVcDgNhVPJzEdsszk8405232 = -926719829;    double GomhVcDgNhVPJzEdsszk60409783 = -632327701;    double GomhVcDgNhVPJzEdsszk90835104 = -290495608;    double GomhVcDgNhVPJzEdsszk41341778 = -1708696;    double GomhVcDgNhVPJzEdsszk63478021 = -487680292;    double GomhVcDgNhVPJzEdsszk31346422 = 41922520;    double GomhVcDgNhVPJzEdsszk54852415 = -355552329;    double GomhVcDgNhVPJzEdsszk95032131 = -339782226;    double GomhVcDgNhVPJzEdsszk19274116 = -201426269;    double GomhVcDgNhVPJzEdsszk78190856 = 28309173;    double GomhVcDgNhVPJzEdsszk60909401 = -114283070;    double GomhVcDgNhVPJzEdsszk70363796 = -219068172;    double GomhVcDgNhVPJzEdsszk77806212 = -199290281;    double GomhVcDgNhVPJzEdsszk46389134 = -669875540;    double GomhVcDgNhVPJzEdsszk31214994 = -31549569;    double GomhVcDgNhVPJzEdsszk75165688 = -672780104;    double GomhVcDgNhVPJzEdsszk63157371 = -356882642;    double GomhVcDgNhVPJzEdsszk81011018 = 73009736;    double GomhVcDgNhVPJzEdsszk99303622 = -842514569;    double GomhVcDgNhVPJzEdsszk63773776 = -306342624;    double GomhVcDgNhVPJzEdsszk5846692 = -215192348;    double GomhVcDgNhVPJzEdsszk34103074 = -854784746;    double GomhVcDgNhVPJzEdsszk61426498 = 97523424;    double GomhVcDgNhVPJzEdsszk79655948 = -714073758;    double GomhVcDgNhVPJzEdsszk30208555 = -497948496;    double GomhVcDgNhVPJzEdsszk79779493 = -906845199;    double GomhVcDgNhVPJzEdsszk7569847 = -902647111;    double GomhVcDgNhVPJzEdsszk13038667 = -967742312;    double GomhVcDgNhVPJzEdsszk65143943 = -812679412;    double GomhVcDgNhVPJzEdsszk65057346 = -666965447;    double GomhVcDgNhVPJzEdsszk11919876 = -16053399;    double GomhVcDgNhVPJzEdsszk97930572 = -915535728;    double GomhVcDgNhVPJzEdsszk27938985 = -330942063;    double GomhVcDgNhVPJzEdsszk27109053 = -338926305;    double GomhVcDgNhVPJzEdsszk97348998 = -180939175;    double GomhVcDgNhVPJzEdsszk92278616 = -315438930;    double GomhVcDgNhVPJzEdsszk88938113 = -675181960;    double GomhVcDgNhVPJzEdsszk32720166 = 18542575;    double GomhVcDgNhVPJzEdsszk30888514 = -56559370;    double GomhVcDgNhVPJzEdsszk53025527 = -503023489;    double GomhVcDgNhVPJzEdsszk37655829 = -509124260;    double GomhVcDgNhVPJzEdsszk23142595 = -652258306;    double GomhVcDgNhVPJzEdsszk40320636 = -377116058;    double GomhVcDgNhVPJzEdsszk33437015 = -368163840;    double GomhVcDgNhVPJzEdsszk41268584 = -532201683;    double GomhVcDgNhVPJzEdsszk49086646 = -754683188;    double GomhVcDgNhVPJzEdsszk84121914 = -945361193;    double GomhVcDgNhVPJzEdsszk13540535 = 7331905;    double GomhVcDgNhVPJzEdsszk29690132 = -349360578;    double GomhVcDgNhVPJzEdsszk32802094 = 94518565;    double GomhVcDgNhVPJzEdsszk25157434 = -588854484;    double GomhVcDgNhVPJzEdsszk8708656 = 52788729;    double GomhVcDgNhVPJzEdsszk6297894 = -210342897;    double GomhVcDgNhVPJzEdsszk98715560 = -954831003;    double GomhVcDgNhVPJzEdsszk76186749 = -923620346;    double GomhVcDgNhVPJzEdsszk74302158 = 28064916;    double GomhVcDgNhVPJzEdsszk98983285 = -629851125;    double GomhVcDgNhVPJzEdsszk11179157 = -576421850;    double GomhVcDgNhVPJzEdsszk11133224 = -503760201;    double GomhVcDgNhVPJzEdsszk83698527 = -580835094;    double GomhVcDgNhVPJzEdsszk23776576 = -55430369;    double GomhVcDgNhVPJzEdsszk41813749 = -387810017;    double GomhVcDgNhVPJzEdsszk29888189 = -527102814;    double GomhVcDgNhVPJzEdsszk54216769 = -534460822;    double GomhVcDgNhVPJzEdsszk66270981 = -955637428;    double GomhVcDgNhVPJzEdsszk62978829 = -533327072;     GomhVcDgNhVPJzEdsszk93697825 = GomhVcDgNhVPJzEdsszk73054368;     GomhVcDgNhVPJzEdsszk73054368 = GomhVcDgNhVPJzEdsszk81833933;     GomhVcDgNhVPJzEdsszk81833933 = GomhVcDgNhVPJzEdsszk49658996;     GomhVcDgNhVPJzEdsszk49658996 = GomhVcDgNhVPJzEdsszk41707620;     GomhVcDgNhVPJzEdsszk41707620 = GomhVcDgNhVPJzEdsszk67499516;     GomhVcDgNhVPJzEdsszk67499516 = GomhVcDgNhVPJzEdsszk73943791;     GomhVcDgNhVPJzEdsszk73943791 = GomhVcDgNhVPJzEdsszk73111726;     GomhVcDgNhVPJzEdsszk73111726 = GomhVcDgNhVPJzEdsszk53372151;     GomhVcDgNhVPJzEdsszk53372151 = GomhVcDgNhVPJzEdsszk1639549;     GomhVcDgNhVPJzEdsszk1639549 = GomhVcDgNhVPJzEdsszk3532088;     GomhVcDgNhVPJzEdsszk3532088 = GomhVcDgNhVPJzEdsszk90428321;     GomhVcDgNhVPJzEdsszk90428321 = GomhVcDgNhVPJzEdsszk9142494;     GomhVcDgNhVPJzEdsszk9142494 = GomhVcDgNhVPJzEdsszk5754231;     GomhVcDgNhVPJzEdsszk5754231 = GomhVcDgNhVPJzEdsszk52688400;     GomhVcDgNhVPJzEdsszk52688400 = GomhVcDgNhVPJzEdsszk79773218;     GomhVcDgNhVPJzEdsszk79773218 = GomhVcDgNhVPJzEdsszk74061943;     GomhVcDgNhVPJzEdsszk74061943 = GomhVcDgNhVPJzEdsszk94366535;     GomhVcDgNhVPJzEdsszk94366535 = GomhVcDgNhVPJzEdsszk84371948;     GomhVcDgNhVPJzEdsszk84371948 = GomhVcDgNhVPJzEdsszk92508243;     GomhVcDgNhVPJzEdsszk92508243 = GomhVcDgNhVPJzEdsszk18174727;     GomhVcDgNhVPJzEdsszk18174727 = GomhVcDgNhVPJzEdsszk59594752;     GomhVcDgNhVPJzEdsszk59594752 = GomhVcDgNhVPJzEdsszk11627871;     GomhVcDgNhVPJzEdsszk11627871 = GomhVcDgNhVPJzEdsszk2177986;     GomhVcDgNhVPJzEdsszk2177986 = GomhVcDgNhVPJzEdsszk19450442;     GomhVcDgNhVPJzEdsszk19450442 = GomhVcDgNhVPJzEdsszk61928127;     GomhVcDgNhVPJzEdsszk61928127 = GomhVcDgNhVPJzEdsszk59929669;     GomhVcDgNhVPJzEdsszk59929669 = GomhVcDgNhVPJzEdsszk60905125;     GomhVcDgNhVPJzEdsszk60905125 = GomhVcDgNhVPJzEdsszk7967783;     GomhVcDgNhVPJzEdsszk7967783 = GomhVcDgNhVPJzEdsszk88314805;     GomhVcDgNhVPJzEdsszk88314805 = GomhVcDgNhVPJzEdsszk89719673;     GomhVcDgNhVPJzEdsszk89719673 = GomhVcDgNhVPJzEdsszk5601516;     GomhVcDgNhVPJzEdsszk5601516 = GomhVcDgNhVPJzEdsszk62489336;     GomhVcDgNhVPJzEdsszk62489336 = GomhVcDgNhVPJzEdsszk82033441;     GomhVcDgNhVPJzEdsszk82033441 = GomhVcDgNhVPJzEdsszk8405232;     GomhVcDgNhVPJzEdsszk8405232 = GomhVcDgNhVPJzEdsszk60409783;     GomhVcDgNhVPJzEdsszk60409783 = GomhVcDgNhVPJzEdsszk90835104;     GomhVcDgNhVPJzEdsszk90835104 = GomhVcDgNhVPJzEdsszk41341778;     GomhVcDgNhVPJzEdsszk41341778 = GomhVcDgNhVPJzEdsszk63478021;     GomhVcDgNhVPJzEdsszk63478021 = GomhVcDgNhVPJzEdsszk31346422;     GomhVcDgNhVPJzEdsszk31346422 = GomhVcDgNhVPJzEdsszk54852415;     GomhVcDgNhVPJzEdsszk54852415 = GomhVcDgNhVPJzEdsszk95032131;     GomhVcDgNhVPJzEdsszk95032131 = GomhVcDgNhVPJzEdsszk19274116;     GomhVcDgNhVPJzEdsszk19274116 = GomhVcDgNhVPJzEdsszk78190856;     GomhVcDgNhVPJzEdsszk78190856 = GomhVcDgNhVPJzEdsszk60909401;     GomhVcDgNhVPJzEdsszk60909401 = GomhVcDgNhVPJzEdsszk70363796;     GomhVcDgNhVPJzEdsszk70363796 = GomhVcDgNhVPJzEdsszk77806212;     GomhVcDgNhVPJzEdsszk77806212 = GomhVcDgNhVPJzEdsszk46389134;     GomhVcDgNhVPJzEdsszk46389134 = GomhVcDgNhVPJzEdsszk31214994;     GomhVcDgNhVPJzEdsszk31214994 = GomhVcDgNhVPJzEdsszk75165688;     GomhVcDgNhVPJzEdsszk75165688 = GomhVcDgNhVPJzEdsszk63157371;     GomhVcDgNhVPJzEdsszk63157371 = GomhVcDgNhVPJzEdsszk81011018;     GomhVcDgNhVPJzEdsszk81011018 = GomhVcDgNhVPJzEdsszk99303622;     GomhVcDgNhVPJzEdsszk99303622 = GomhVcDgNhVPJzEdsszk63773776;     GomhVcDgNhVPJzEdsszk63773776 = GomhVcDgNhVPJzEdsszk5846692;     GomhVcDgNhVPJzEdsszk5846692 = GomhVcDgNhVPJzEdsszk34103074;     GomhVcDgNhVPJzEdsszk34103074 = GomhVcDgNhVPJzEdsszk61426498;     GomhVcDgNhVPJzEdsszk61426498 = GomhVcDgNhVPJzEdsszk79655948;     GomhVcDgNhVPJzEdsszk79655948 = GomhVcDgNhVPJzEdsszk30208555;     GomhVcDgNhVPJzEdsszk30208555 = GomhVcDgNhVPJzEdsszk79779493;     GomhVcDgNhVPJzEdsszk79779493 = GomhVcDgNhVPJzEdsszk7569847;     GomhVcDgNhVPJzEdsszk7569847 = GomhVcDgNhVPJzEdsszk13038667;     GomhVcDgNhVPJzEdsszk13038667 = GomhVcDgNhVPJzEdsszk65143943;     GomhVcDgNhVPJzEdsszk65143943 = GomhVcDgNhVPJzEdsszk65057346;     GomhVcDgNhVPJzEdsszk65057346 = GomhVcDgNhVPJzEdsszk11919876;     GomhVcDgNhVPJzEdsszk11919876 = GomhVcDgNhVPJzEdsszk97930572;     GomhVcDgNhVPJzEdsszk97930572 = GomhVcDgNhVPJzEdsszk27938985;     GomhVcDgNhVPJzEdsszk27938985 = GomhVcDgNhVPJzEdsszk27109053;     GomhVcDgNhVPJzEdsszk27109053 = GomhVcDgNhVPJzEdsszk97348998;     GomhVcDgNhVPJzEdsszk97348998 = GomhVcDgNhVPJzEdsszk92278616;     GomhVcDgNhVPJzEdsszk92278616 = GomhVcDgNhVPJzEdsszk88938113;     GomhVcDgNhVPJzEdsszk88938113 = GomhVcDgNhVPJzEdsszk32720166;     GomhVcDgNhVPJzEdsszk32720166 = GomhVcDgNhVPJzEdsszk30888514;     GomhVcDgNhVPJzEdsszk30888514 = GomhVcDgNhVPJzEdsszk53025527;     GomhVcDgNhVPJzEdsszk53025527 = GomhVcDgNhVPJzEdsszk37655829;     GomhVcDgNhVPJzEdsszk37655829 = GomhVcDgNhVPJzEdsszk23142595;     GomhVcDgNhVPJzEdsszk23142595 = GomhVcDgNhVPJzEdsszk40320636;     GomhVcDgNhVPJzEdsszk40320636 = GomhVcDgNhVPJzEdsszk33437015;     GomhVcDgNhVPJzEdsszk33437015 = GomhVcDgNhVPJzEdsszk41268584;     GomhVcDgNhVPJzEdsszk41268584 = GomhVcDgNhVPJzEdsszk49086646;     GomhVcDgNhVPJzEdsszk49086646 = GomhVcDgNhVPJzEdsszk84121914;     GomhVcDgNhVPJzEdsszk84121914 = GomhVcDgNhVPJzEdsszk13540535;     GomhVcDgNhVPJzEdsszk13540535 = GomhVcDgNhVPJzEdsszk29690132;     GomhVcDgNhVPJzEdsszk29690132 = GomhVcDgNhVPJzEdsszk32802094;     GomhVcDgNhVPJzEdsszk32802094 = GomhVcDgNhVPJzEdsszk25157434;     GomhVcDgNhVPJzEdsszk25157434 = GomhVcDgNhVPJzEdsszk8708656;     GomhVcDgNhVPJzEdsszk8708656 = GomhVcDgNhVPJzEdsszk6297894;     GomhVcDgNhVPJzEdsszk6297894 = GomhVcDgNhVPJzEdsszk98715560;     GomhVcDgNhVPJzEdsszk98715560 = GomhVcDgNhVPJzEdsszk76186749;     GomhVcDgNhVPJzEdsszk76186749 = GomhVcDgNhVPJzEdsszk74302158;     GomhVcDgNhVPJzEdsszk74302158 = GomhVcDgNhVPJzEdsszk98983285;     GomhVcDgNhVPJzEdsszk98983285 = GomhVcDgNhVPJzEdsszk11179157;     GomhVcDgNhVPJzEdsszk11179157 = GomhVcDgNhVPJzEdsszk11133224;     GomhVcDgNhVPJzEdsszk11133224 = GomhVcDgNhVPJzEdsszk83698527;     GomhVcDgNhVPJzEdsszk83698527 = GomhVcDgNhVPJzEdsszk23776576;     GomhVcDgNhVPJzEdsszk23776576 = GomhVcDgNhVPJzEdsszk41813749;     GomhVcDgNhVPJzEdsszk41813749 = GomhVcDgNhVPJzEdsszk29888189;     GomhVcDgNhVPJzEdsszk29888189 = GomhVcDgNhVPJzEdsszk54216769;     GomhVcDgNhVPJzEdsszk54216769 = GomhVcDgNhVPJzEdsszk66270981;     GomhVcDgNhVPJzEdsszk66270981 = GomhVcDgNhVPJzEdsszk62978829;     GomhVcDgNhVPJzEdsszk62978829 = GomhVcDgNhVPJzEdsszk93697825;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lvEXADCaChHcyWMqNkJh81932102() {     double eIZudChSaRWgHzhlqPIk87015822 = -255236551;    double eIZudChSaRWgHzhlqPIk6801393 = -66787259;    double eIZudChSaRWgHzhlqPIk82749741 = -117289277;    double eIZudChSaRWgHzhlqPIk38952673 = -579327869;    double eIZudChSaRWgHzhlqPIk34772209 = 82959279;    double eIZudChSaRWgHzhlqPIk85217512 = -551629291;    double eIZudChSaRWgHzhlqPIk50319671 = -551018299;    double eIZudChSaRWgHzhlqPIk34580606 = -841651241;    double eIZudChSaRWgHzhlqPIk88090164 = 53730715;    double eIZudChSaRWgHzhlqPIk29929531 = -988756520;    double eIZudChSaRWgHzhlqPIk76796206 = -750088908;    double eIZudChSaRWgHzhlqPIk18884213 = -100145800;    double eIZudChSaRWgHzhlqPIk88914136 = -57164379;    double eIZudChSaRWgHzhlqPIk46962269 = -997788476;    double eIZudChSaRWgHzhlqPIk15468492 = -656668669;    double eIZudChSaRWgHzhlqPIk41982638 = -53377023;    double eIZudChSaRWgHzhlqPIk23851648 = -675165906;    double eIZudChSaRWgHzhlqPIk53862269 = -448695245;    double eIZudChSaRWgHzhlqPIk85897700 = -93365403;    double eIZudChSaRWgHzhlqPIk14551439 = -846384271;    double eIZudChSaRWgHzhlqPIk85892711 = -977144470;    double eIZudChSaRWgHzhlqPIk73660488 = -336597461;    double eIZudChSaRWgHzhlqPIk82215160 = -73283942;    double eIZudChSaRWgHzhlqPIk28963447 = -370285773;    double eIZudChSaRWgHzhlqPIk14141213 = -989893175;    double eIZudChSaRWgHzhlqPIk38479311 = -149287455;    double eIZudChSaRWgHzhlqPIk87417074 = -631098983;    double eIZudChSaRWgHzhlqPIk77466375 = -490974972;    double eIZudChSaRWgHzhlqPIk82839879 = -878567632;    double eIZudChSaRWgHzhlqPIk56359066 = -584184586;    double eIZudChSaRWgHzhlqPIk4351585 = -385292192;    double eIZudChSaRWgHzhlqPIk69341028 = -567228125;    double eIZudChSaRWgHzhlqPIk97059182 = -681820644;    double eIZudChSaRWgHzhlqPIk353307 = -838963706;    double eIZudChSaRWgHzhlqPIk51535427 = -374048903;    double eIZudChSaRWgHzhlqPIk19567961 = -117322197;    double eIZudChSaRWgHzhlqPIk59215714 = -653528883;    double eIZudChSaRWgHzhlqPIk57563326 = -944119972;    double eIZudChSaRWgHzhlqPIk1940083 = -965116044;    double eIZudChSaRWgHzhlqPIk36329416 = -905963229;    double eIZudChSaRWgHzhlqPIk74801666 = -221240027;    double eIZudChSaRWgHzhlqPIk90875454 = 802666;    double eIZudChSaRWgHzhlqPIk83024344 = -277057916;    double eIZudChSaRWgHzhlqPIk39433855 = -963790409;    double eIZudChSaRWgHzhlqPIk60889481 = -829930067;    double eIZudChSaRWgHzhlqPIk89530798 = -855055190;    double eIZudChSaRWgHzhlqPIk73398352 = -783112011;    double eIZudChSaRWgHzhlqPIk75673270 = -641941005;    double eIZudChSaRWgHzhlqPIk34143452 = -181212305;    double eIZudChSaRWgHzhlqPIk81217767 = -422804222;    double eIZudChSaRWgHzhlqPIk7049296 = -203402019;    double eIZudChSaRWgHzhlqPIk92738705 = -20822743;    double eIZudChSaRWgHzhlqPIk63963626 = -242408890;    double eIZudChSaRWgHzhlqPIk35543473 = 98799270;    double eIZudChSaRWgHzhlqPIk96039391 = -435078200;    double eIZudChSaRWgHzhlqPIk13355334 = -918639091;    double eIZudChSaRWgHzhlqPIk24586233 = -993503317;    double eIZudChSaRWgHzhlqPIk53786295 = -747003505;    double eIZudChSaRWgHzhlqPIk24811461 = -589434695;    double eIZudChSaRWgHzhlqPIk96292897 = -767753267;    double eIZudChSaRWgHzhlqPIk97800438 = -920530308;    double eIZudChSaRWgHzhlqPIk72853296 = 39956673;    double eIZudChSaRWgHzhlqPIk51740726 = -963083609;    double eIZudChSaRWgHzhlqPIk31731098 = -362084700;    double eIZudChSaRWgHzhlqPIk25577946 = -503464328;    double eIZudChSaRWgHzhlqPIk7455178 = -82860783;    double eIZudChSaRWgHzhlqPIk21825031 = -418325156;    double eIZudChSaRWgHzhlqPIk88560830 = -218200673;    double eIZudChSaRWgHzhlqPIk95426841 = -523739573;    double eIZudChSaRWgHzhlqPIk95900530 = -439346473;    double eIZudChSaRWgHzhlqPIk82766923 = -399848140;    double eIZudChSaRWgHzhlqPIk66288321 = -731045934;    double eIZudChSaRWgHzhlqPIk51922187 = -483579202;    double eIZudChSaRWgHzhlqPIk49568284 = -187402174;    double eIZudChSaRWgHzhlqPIk39749772 = -525144245;    double eIZudChSaRWgHzhlqPIk95017256 = -877947137;    double eIZudChSaRWgHzhlqPIk90636143 = 40460454;    double eIZudChSaRWgHzhlqPIk42781305 = -109493534;    double eIZudChSaRWgHzhlqPIk68073966 = -540355707;    double eIZudChSaRWgHzhlqPIk24610414 = -34837985;    double eIZudChSaRWgHzhlqPIk65080959 = -366175444;    double eIZudChSaRWgHzhlqPIk11743804 = -989157978;    double eIZudChSaRWgHzhlqPIk43322924 = -209762667;    double eIZudChSaRWgHzhlqPIk1622113 = -355763411;    double eIZudChSaRWgHzhlqPIk49309771 = -280782567;    double eIZudChSaRWgHzhlqPIk11612880 = -264469450;    double eIZudChSaRWgHzhlqPIk5377403 = -224819236;    double eIZudChSaRWgHzhlqPIk61515709 = -680619915;    double eIZudChSaRWgHzhlqPIk4313916 = -303885507;    double eIZudChSaRWgHzhlqPIk38180093 = -455409813;    double eIZudChSaRWgHzhlqPIk94981728 = -123818880;    double eIZudChSaRWgHzhlqPIk5429420 = -906525379;    double eIZudChSaRWgHzhlqPIk32751866 = -254685278;    double eIZudChSaRWgHzhlqPIk5647185 = -97362778;    double eIZudChSaRWgHzhlqPIk38528978 = -985432922;    double eIZudChSaRWgHzhlqPIk1948371 = -161196700;    double eIZudChSaRWgHzhlqPIk39134729 = -36113726;    double eIZudChSaRWgHzhlqPIk51293246 = -914973217;    double eIZudChSaRWgHzhlqPIk13855909 = -360326081;    double eIZudChSaRWgHzhlqPIk53434303 = -255236551;     eIZudChSaRWgHzhlqPIk87015822 = eIZudChSaRWgHzhlqPIk6801393;     eIZudChSaRWgHzhlqPIk6801393 = eIZudChSaRWgHzhlqPIk82749741;     eIZudChSaRWgHzhlqPIk82749741 = eIZudChSaRWgHzhlqPIk38952673;     eIZudChSaRWgHzhlqPIk38952673 = eIZudChSaRWgHzhlqPIk34772209;     eIZudChSaRWgHzhlqPIk34772209 = eIZudChSaRWgHzhlqPIk85217512;     eIZudChSaRWgHzhlqPIk85217512 = eIZudChSaRWgHzhlqPIk50319671;     eIZudChSaRWgHzhlqPIk50319671 = eIZudChSaRWgHzhlqPIk34580606;     eIZudChSaRWgHzhlqPIk34580606 = eIZudChSaRWgHzhlqPIk88090164;     eIZudChSaRWgHzhlqPIk88090164 = eIZudChSaRWgHzhlqPIk29929531;     eIZudChSaRWgHzhlqPIk29929531 = eIZudChSaRWgHzhlqPIk76796206;     eIZudChSaRWgHzhlqPIk76796206 = eIZudChSaRWgHzhlqPIk18884213;     eIZudChSaRWgHzhlqPIk18884213 = eIZudChSaRWgHzhlqPIk88914136;     eIZudChSaRWgHzhlqPIk88914136 = eIZudChSaRWgHzhlqPIk46962269;     eIZudChSaRWgHzhlqPIk46962269 = eIZudChSaRWgHzhlqPIk15468492;     eIZudChSaRWgHzhlqPIk15468492 = eIZudChSaRWgHzhlqPIk41982638;     eIZudChSaRWgHzhlqPIk41982638 = eIZudChSaRWgHzhlqPIk23851648;     eIZudChSaRWgHzhlqPIk23851648 = eIZudChSaRWgHzhlqPIk53862269;     eIZudChSaRWgHzhlqPIk53862269 = eIZudChSaRWgHzhlqPIk85897700;     eIZudChSaRWgHzhlqPIk85897700 = eIZudChSaRWgHzhlqPIk14551439;     eIZudChSaRWgHzhlqPIk14551439 = eIZudChSaRWgHzhlqPIk85892711;     eIZudChSaRWgHzhlqPIk85892711 = eIZudChSaRWgHzhlqPIk73660488;     eIZudChSaRWgHzhlqPIk73660488 = eIZudChSaRWgHzhlqPIk82215160;     eIZudChSaRWgHzhlqPIk82215160 = eIZudChSaRWgHzhlqPIk28963447;     eIZudChSaRWgHzhlqPIk28963447 = eIZudChSaRWgHzhlqPIk14141213;     eIZudChSaRWgHzhlqPIk14141213 = eIZudChSaRWgHzhlqPIk38479311;     eIZudChSaRWgHzhlqPIk38479311 = eIZudChSaRWgHzhlqPIk87417074;     eIZudChSaRWgHzhlqPIk87417074 = eIZudChSaRWgHzhlqPIk77466375;     eIZudChSaRWgHzhlqPIk77466375 = eIZudChSaRWgHzhlqPIk82839879;     eIZudChSaRWgHzhlqPIk82839879 = eIZudChSaRWgHzhlqPIk56359066;     eIZudChSaRWgHzhlqPIk56359066 = eIZudChSaRWgHzhlqPIk4351585;     eIZudChSaRWgHzhlqPIk4351585 = eIZudChSaRWgHzhlqPIk69341028;     eIZudChSaRWgHzhlqPIk69341028 = eIZudChSaRWgHzhlqPIk97059182;     eIZudChSaRWgHzhlqPIk97059182 = eIZudChSaRWgHzhlqPIk353307;     eIZudChSaRWgHzhlqPIk353307 = eIZudChSaRWgHzhlqPIk51535427;     eIZudChSaRWgHzhlqPIk51535427 = eIZudChSaRWgHzhlqPIk19567961;     eIZudChSaRWgHzhlqPIk19567961 = eIZudChSaRWgHzhlqPIk59215714;     eIZudChSaRWgHzhlqPIk59215714 = eIZudChSaRWgHzhlqPIk57563326;     eIZudChSaRWgHzhlqPIk57563326 = eIZudChSaRWgHzhlqPIk1940083;     eIZudChSaRWgHzhlqPIk1940083 = eIZudChSaRWgHzhlqPIk36329416;     eIZudChSaRWgHzhlqPIk36329416 = eIZudChSaRWgHzhlqPIk74801666;     eIZudChSaRWgHzhlqPIk74801666 = eIZudChSaRWgHzhlqPIk90875454;     eIZudChSaRWgHzhlqPIk90875454 = eIZudChSaRWgHzhlqPIk83024344;     eIZudChSaRWgHzhlqPIk83024344 = eIZudChSaRWgHzhlqPIk39433855;     eIZudChSaRWgHzhlqPIk39433855 = eIZudChSaRWgHzhlqPIk60889481;     eIZudChSaRWgHzhlqPIk60889481 = eIZudChSaRWgHzhlqPIk89530798;     eIZudChSaRWgHzhlqPIk89530798 = eIZudChSaRWgHzhlqPIk73398352;     eIZudChSaRWgHzhlqPIk73398352 = eIZudChSaRWgHzhlqPIk75673270;     eIZudChSaRWgHzhlqPIk75673270 = eIZudChSaRWgHzhlqPIk34143452;     eIZudChSaRWgHzhlqPIk34143452 = eIZudChSaRWgHzhlqPIk81217767;     eIZudChSaRWgHzhlqPIk81217767 = eIZudChSaRWgHzhlqPIk7049296;     eIZudChSaRWgHzhlqPIk7049296 = eIZudChSaRWgHzhlqPIk92738705;     eIZudChSaRWgHzhlqPIk92738705 = eIZudChSaRWgHzhlqPIk63963626;     eIZudChSaRWgHzhlqPIk63963626 = eIZudChSaRWgHzhlqPIk35543473;     eIZudChSaRWgHzhlqPIk35543473 = eIZudChSaRWgHzhlqPIk96039391;     eIZudChSaRWgHzhlqPIk96039391 = eIZudChSaRWgHzhlqPIk13355334;     eIZudChSaRWgHzhlqPIk13355334 = eIZudChSaRWgHzhlqPIk24586233;     eIZudChSaRWgHzhlqPIk24586233 = eIZudChSaRWgHzhlqPIk53786295;     eIZudChSaRWgHzhlqPIk53786295 = eIZudChSaRWgHzhlqPIk24811461;     eIZudChSaRWgHzhlqPIk24811461 = eIZudChSaRWgHzhlqPIk96292897;     eIZudChSaRWgHzhlqPIk96292897 = eIZudChSaRWgHzhlqPIk97800438;     eIZudChSaRWgHzhlqPIk97800438 = eIZudChSaRWgHzhlqPIk72853296;     eIZudChSaRWgHzhlqPIk72853296 = eIZudChSaRWgHzhlqPIk51740726;     eIZudChSaRWgHzhlqPIk51740726 = eIZudChSaRWgHzhlqPIk31731098;     eIZudChSaRWgHzhlqPIk31731098 = eIZudChSaRWgHzhlqPIk25577946;     eIZudChSaRWgHzhlqPIk25577946 = eIZudChSaRWgHzhlqPIk7455178;     eIZudChSaRWgHzhlqPIk7455178 = eIZudChSaRWgHzhlqPIk21825031;     eIZudChSaRWgHzhlqPIk21825031 = eIZudChSaRWgHzhlqPIk88560830;     eIZudChSaRWgHzhlqPIk88560830 = eIZudChSaRWgHzhlqPIk95426841;     eIZudChSaRWgHzhlqPIk95426841 = eIZudChSaRWgHzhlqPIk95900530;     eIZudChSaRWgHzhlqPIk95900530 = eIZudChSaRWgHzhlqPIk82766923;     eIZudChSaRWgHzhlqPIk82766923 = eIZudChSaRWgHzhlqPIk66288321;     eIZudChSaRWgHzhlqPIk66288321 = eIZudChSaRWgHzhlqPIk51922187;     eIZudChSaRWgHzhlqPIk51922187 = eIZudChSaRWgHzhlqPIk49568284;     eIZudChSaRWgHzhlqPIk49568284 = eIZudChSaRWgHzhlqPIk39749772;     eIZudChSaRWgHzhlqPIk39749772 = eIZudChSaRWgHzhlqPIk95017256;     eIZudChSaRWgHzhlqPIk95017256 = eIZudChSaRWgHzhlqPIk90636143;     eIZudChSaRWgHzhlqPIk90636143 = eIZudChSaRWgHzhlqPIk42781305;     eIZudChSaRWgHzhlqPIk42781305 = eIZudChSaRWgHzhlqPIk68073966;     eIZudChSaRWgHzhlqPIk68073966 = eIZudChSaRWgHzhlqPIk24610414;     eIZudChSaRWgHzhlqPIk24610414 = eIZudChSaRWgHzhlqPIk65080959;     eIZudChSaRWgHzhlqPIk65080959 = eIZudChSaRWgHzhlqPIk11743804;     eIZudChSaRWgHzhlqPIk11743804 = eIZudChSaRWgHzhlqPIk43322924;     eIZudChSaRWgHzhlqPIk43322924 = eIZudChSaRWgHzhlqPIk1622113;     eIZudChSaRWgHzhlqPIk1622113 = eIZudChSaRWgHzhlqPIk49309771;     eIZudChSaRWgHzhlqPIk49309771 = eIZudChSaRWgHzhlqPIk11612880;     eIZudChSaRWgHzhlqPIk11612880 = eIZudChSaRWgHzhlqPIk5377403;     eIZudChSaRWgHzhlqPIk5377403 = eIZudChSaRWgHzhlqPIk61515709;     eIZudChSaRWgHzhlqPIk61515709 = eIZudChSaRWgHzhlqPIk4313916;     eIZudChSaRWgHzhlqPIk4313916 = eIZudChSaRWgHzhlqPIk38180093;     eIZudChSaRWgHzhlqPIk38180093 = eIZudChSaRWgHzhlqPIk94981728;     eIZudChSaRWgHzhlqPIk94981728 = eIZudChSaRWgHzhlqPIk5429420;     eIZudChSaRWgHzhlqPIk5429420 = eIZudChSaRWgHzhlqPIk32751866;     eIZudChSaRWgHzhlqPIk32751866 = eIZudChSaRWgHzhlqPIk5647185;     eIZudChSaRWgHzhlqPIk5647185 = eIZudChSaRWgHzhlqPIk38528978;     eIZudChSaRWgHzhlqPIk38528978 = eIZudChSaRWgHzhlqPIk1948371;     eIZudChSaRWgHzhlqPIk1948371 = eIZudChSaRWgHzhlqPIk39134729;     eIZudChSaRWgHzhlqPIk39134729 = eIZudChSaRWgHzhlqPIk51293246;     eIZudChSaRWgHzhlqPIk51293246 = eIZudChSaRWgHzhlqPIk13855909;     eIZudChSaRWgHzhlqPIk13855909 = eIZudChSaRWgHzhlqPIk53434303;     eIZudChSaRWgHzhlqPIk53434303 = eIZudChSaRWgHzhlqPIk87015822;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BROpkChhObwEGcPJaCrH9526359() {     double RvSRVWKYfBXFBczsRBWo12601100 = -780269652;    double RvSRVWKYfBXFBczsRBWo3198536 = -322265036;    double RvSRVWKYfBXFBczsRBWo15628918 = 80249186;    double RvSRVWKYfBXFBczsRBWo84674602 = -586650762;    double RvSRVWKYfBXFBczsRBWo4114214 = 36036992;    double RvSRVWKYfBXFBczsRBWo2226789 = -706610294;    double RvSRVWKYfBXFBczsRBWo55640516 = -33289506;    double RvSRVWKYfBXFBczsRBWo49590730 = -122333120;    double RvSRVWKYfBXFBczsRBWo13419457 = -442493330;    double RvSRVWKYfBXFBczsRBWo49087913 = -223318043;    double RvSRVWKYfBXFBczsRBWo91129760 = 39483205;    double RvSRVWKYfBXFBczsRBWo54201869 = -543854706;    double RvSRVWKYfBXFBczsRBWo65494914 = -921412745;    double RvSRVWKYfBXFBczsRBWo38521986 = -972312769;    double RvSRVWKYfBXFBczsRBWo71737379 = -545214627;    double RvSRVWKYfBXFBczsRBWo49703681 = -489568500;    double RvSRVWKYfBXFBczsRBWo23649765 = -363485700;    double RvSRVWKYfBXFBczsRBWo74978173 = -964972606;    double RvSRVWKYfBXFBczsRBWo47362421 = -876339260;    double RvSRVWKYfBXFBczsRBWo75712905 = -820823647;    double RvSRVWKYfBXFBczsRBWo86901976 = -338844251;    double RvSRVWKYfBXFBczsRBWo75163595 = -756330391;    double RvSRVWKYfBXFBczsRBWo85978958 = -117376047;    double RvSRVWKYfBXFBczsRBWo30677489 = -229134753;    double RvSRVWKYfBXFBczsRBWo69044351 = -557389317;    double RvSRVWKYfBXFBczsRBWo67968447 = -813737996;    double RvSRVWKYfBXFBczsRBWo69804983 = -196912118;    double RvSRVWKYfBXFBczsRBWo41365176 = -720637205;    double RvSRVWKYfBXFBczsRBWo90717091 = -14861483;    double RvSRVWKYfBXFBczsRBWo9681557 = -493094148;    double RvSRVWKYfBXFBczsRBWo26398219 = -163939223;    double RvSRVWKYfBXFBczsRBWo6530961 = -841023959;    double RvSRVWKYfBXFBczsRBWo18246234 = -29641781;    double RvSRVWKYfBXFBczsRBWo53940378 = -895108678;    double RvSRVWKYfBXFBczsRBWo52940415 = 24515185;    double RvSRVWKYfBXFBczsRBWo24359811 = -634916913;    double RvSRVWKYfBXFBczsRBWo8861100 = -34040828;    double RvSRVWKYfBXFBczsRBWo33136013 = -792834798;    double RvSRVWKYfBXFBczsRBWo42863661 = -323454366;    double RvSRVWKYfBXFBczsRBWo45113090 = -275933548;    double RvSRVWKYfBXFBczsRBWo21952948 = -796300216;    double RvSRVWKYfBXFBczsRBWo34885045 = -508235838;    double RvSRVWKYfBXFBczsRBWo32224564 = -305664298;    double RvSRVWKYfBXFBczsRBWo10227135 = -332206008;    double RvSRVWKYfBXFBczsRBWo52870357 = -768951185;    double RvSRVWKYfBXFBczsRBWo55931121 = -277602728;    double RvSRVWKYfBXFBczsRBWo73166806 = -815580871;    double RvSRVWKYfBXFBczsRBWo83786041 = -835123852;    double RvSRVWKYfBXFBczsRBWo84954771 = -280888532;    double RvSRVWKYfBXFBczsRBWo35027762 = -270827375;    double RvSRVWKYfBXFBczsRBWo17185543 = -452060621;    double RvSRVWKYfBXFBczsRBWo19997285 = -198901923;    double RvSRVWKYfBXFBczsRBWo82037229 = -810307439;    double RvSRVWKYfBXFBczsRBWo4442383 = -216264511;    double RvSRVWKYfBXFBczsRBWo22624383 = -910168617;    double RvSRVWKYfBXFBczsRBWo37437505 = 76060738;    double RvSRVWKYfBXFBczsRBWo17219578 = -104888989;    double RvSRVWKYfBXFBczsRBWo84951428 = -690616061;    double RvSRVWKYfBXFBczsRBWo15630252 = 70738555;    double RvSRVWKYfBXFBczsRBWo36145766 = -150225013;    double RvSRVWKYfBXFBczsRBWo32421806 = -409698177;    double RvSRVWKYfBXFBczsRBWo14275340 = -312652301;    double RvSRVWKYfBXFBczsRBWo58873638 = -7471637;    double RvSRVWKYfBXFBczsRBWo3737900 = -949399183;    double RvSRVWKYfBXFBczsRBWo22689694 = 40621179;    double RvSRVWKYfBXFBczsRBWo84598799 = -119492836;    double RvSRVWKYfBXFBczsRBWo35955635 = -414212926;    double RvSRVWKYfBXFBczsRBWo11554537 = 73695933;    double RvSRVWKYfBXFBczsRBWo85581570 = -896827955;    double RvSRVWKYfBXFBczsRBWo47377568 = -910297715;    double RvSRVWKYfBXFBczsRBWo40842581 = -355527672;    double RvSRVWKYfBXFBczsRBWo90513751 = -570650903;    double RvSRVWKYfBXFBczsRBWo32114513 = -541518241;    double RvSRVWKYfBXFBczsRBWo2249332 = -500405713;    double RvSRVWKYfBXFBczsRBWo53759957 = 75476569;    double RvSRVWKYfBXFBczsRBWo52016932 = -830608414;    double RvSRVWKYfBXFBczsRBWo42939031 = -350666094;    double RvSRVWKYfBXFBczsRBWo75751824 = -785170040;    double RvSRVWKYfBXFBczsRBWo77807132 = -460183569;    double RvSRVWKYfBXFBczsRBWo13113231 = -179786590;    double RvSRVWKYfBXFBczsRBWo94801641 = -998157126;    double RvSRVWKYfBXFBczsRBWo86018941 = -361788266;    double RvSRVWKYfBXFBczsRBWo56410404 = -339748673;    double RvSRVWKYfBXFBczsRBWo55689330 = -744034108;    double RvSRVWKYfBXFBczsRBWo92496014 = 58966473;    double RvSRVWKYfBXFBczsRBWo6400935 = -965037301;    double RvSRVWKYfBXFBczsRBWo24493731 = 69283479;    double RvSRVWKYfBXFBczsRBWo13803852 = -813377270;    double RvSRVWKYfBXFBczsRBWo31315996 = -984940061;    double RvSRVWKYfBXFBczsRBWo15502911 = 48454447;    double RvSRVWKYfBXFBczsRBWo7140234 = -430027924;    double RvSRVWKYfBXFBczsRBWo23909672 = -343424767;    double RvSRVWKYfBXFBczsRBWo17505762 = -763573353;    double RvSRVWKYfBXFBczsRBWo6717895 = -73229354;    double RvSRVWKYfBXFBczsRBWo12691285 = -866235372;    double RvSRVWKYfBXFBczsRBWo7677608 = -383647916;    double RvSRVWKYfBXFBczsRBWo76011406 = -400764201;    double RvSRVWKYfBXFBczsRBWo28486664 = -356265115;    double RvSRVWKYfBXFBczsRBWo87537440 = -272827187;    double RvSRVWKYfBXFBczsRBWo68271557 = -780269652;     RvSRVWKYfBXFBczsRBWo12601100 = RvSRVWKYfBXFBczsRBWo3198536;     RvSRVWKYfBXFBczsRBWo3198536 = RvSRVWKYfBXFBczsRBWo15628918;     RvSRVWKYfBXFBczsRBWo15628918 = RvSRVWKYfBXFBczsRBWo84674602;     RvSRVWKYfBXFBczsRBWo84674602 = RvSRVWKYfBXFBczsRBWo4114214;     RvSRVWKYfBXFBczsRBWo4114214 = RvSRVWKYfBXFBczsRBWo2226789;     RvSRVWKYfBXFBczsRBWo2226789 = RvSRVWKYfBXFBczsRBWo55640516;     RvSRVWKYfBXFBczsRBWo55640516 = RvSRVWKYfBXFBczsRBWo49590730;     RvSRVWKYfBXFBczsRBWo49590730 = RvSRVWKYfBXFBczsRBWo13419457;     RvSRVWKYfBXFBczsRBWo13419457 = RvSRVWKYfBXFBczsRBWo49087913;     RvSRVWKYfBXFBczsRBWo49087913 = RvSRVWKYfBXFBczsRBWo91129760;     RvSRVWKYfBXFBczsRBWo91129760 = RvSRVWKYfBXFBczsRBWo54201869;     RvSRVWKYfBXFBczsRBWo54201869 = RvSRVWKYfBXFBczsRBWo65494914;     RvSRVWKYfBXFBczsRBWo65494914 = RvSRVWKYfBXFBczsRBWo38521986;     RvSRVWKYfBXFBczsRBWo38521986 = RvSRVWKYfBXFBczsRBWo71737379;     RvSRVWKYfBXFBczsRBWo71737379 = RvSRVWKYfBXFBczsRBWo49703681;     RvSRVWKYfBXFBczsRBWo49703681 = RvSRVWKYfBXFBczsRBWo23649765;     RvSRVWKYfBXFBczsRBWo23649765 = RvSRVWKYfBXFBczsRBWo74978173;     RvSRVWKYfBXFBczsRBWo74978173 = RvSRVWKYfBXFBczsRBWo47362421;     RvSRVWKYfBXFBczsRBWo47362421 = RvSRVWKYfBXFBczsRBWo75712905;     RvSRVWKYfBXFBczsRBWo75712905 = RvSRVWKYfBXFBczsRBWo86901976;     RvSRVWKYfBXFBczsRBWo86901976 = RvSRVWKYfBXFBczsRBWo75163595;     RvSRVWKYfBXFBczsRBWo75163595 = RvSRVWKYfBXFBczsRBWo85978958;     RvSRVWKYfBXFBczsRBWo85978958 = RvSRVWKYfBXFBczsRBWo30677489;     RvSRVWKYfBXFBczsRBWo30677489 = RvSRVWKYfBXFBczsRBWo69044351;     RvSRVWKYfBXFBczsRBWo69044351 = RvSRVWKYfBXFBczsRBWo67968447;     RvSRVWKYfBXFBczsRBWo67968447 = RvSRVWKYfBXFBczsRBWo69804983;     RvSRVWKYfBXFBczsRBWo69804983 = RvSRVWKYfBXFBczsRBWo41365176;     RvSRVWKYfBXFBczsRBWo41365176 = RvSRVWKYfBXFBczsRBWo90717091;     RvSRVWKYfBXFBczsRBWo90717091 = RvSRVWKYfBXFBczsRBWo9681557;     RvSRVWKYfBXFBczsRBWo9681557 = RvSRVWKYfBXFBczsRBWo26398219;     RvSRVWKYfBXFBczsRBWo26398219 = RvSRVWKYfBXFBczsRBWo6530961;     RvSRVWKYfBXFBczsRBWo6530961 = RvSRVWKYfBXFBczsRBWo18246234;     RvSRVWKYfBXFBczsRBWo18246234 = RvSRVWKYfBXFBczsRBWo53940378;     RvSRVWKYfBXFBczsRBWo53940378 = RvSRVWKYfBXFBczsRBWo52940415;     RvSRVWKYfBXFBczsRBWo52940415 = RvSRVWKYfBXFBczsRBWo24359811;     RvSRVWKYfBXFBczsRBWo24359811 = RvSRVWKYfBXFBczsRBWo8861100;     RvSRVWKYfBXFBczsRBWo8861100 = RvSRVWKYfBXFBczsRBWo33136013;     RvSRVWKYfBXFBczsRBWo33136013 = RvSRVWKYfBXFBczsRBWo42863661;     RvSRVWKYfBXFBczsRBWo42863661 = RvSRVWKYfBXFBczsRBWo45113090;     RvSRVWKYfBXFBczsRBWo45113090 = RvSRVWKYfBXFBczsRBWo21952948;     RvSRVWKYfBXFBczsRBWo21952948 = RvSRVWKYfBXFBczsRBWo34885045;     RvSRVWKYfBXFBczsRBWo34885045 = RvSRVWKYfBXFBczsRBWo32224564;     RvSRVWKYfBXFBczsRBWo32224564 = RvSRVWKYfBXFBczsRBWo10227135;     RvSRVWKYfBXFBczsRBWo10227135 = RvSRVWKYfBXFBczsRBWo52870357;     RvSRVWKYfBXFBczsRBWo52870357 = RvSRVWKYfBXFBczsRBWo55931121;     RvSRVWKYfBXFBczsRBWo55931121 = RvSRVWKYfBXFBczsRBWo73166806;     RvSRVWKYfBXFBczsRBWo73166806 = RvSRVWKYfBXFBczsRBWo83786041;     RvSRVWKYfBXFBczsRBWo83786041 = RvSRVWKYfBXFBczsRBWo84954771;     RvSRVWKYfBXFBczsRBWo84954771 = RvSRVWKYfBXFBczsRBWo35027762;     RvSRVWKYfBXFBczsRBWo35027762 = RvSRVWKYfBXFBczsRBWo17185543;     RvSRVWKYfBXFBczsRBWo17185543 = RvSRVWKYfBXFBczsRBWo19997285;     RvSRVWKYfBXFBczsRBWo19997285 = RvSRVWKYfBXFBczsRBWo82037229;     RvSRVWKYfBXFBczsRBWo82037229 = RvSRVWKYfBXFBczsRBWo4442383;     RvSRVWKYfBXFBczsRBWo4442383 = RvSRVWKYfBXFBczsRBWo22624383;     RvSRVWKYfBXFBczsRBWo22624383 = RvSRVWKYfBXFBczsRBWo37437505;     RvSRVWKYfBXFBczsRBWo37437505 = RvSRVWKYfBXFBczsRBWo17219578;     RvSRVWKYfBXFBczsRBWo17219578 = RvSRVWKYfBXFBczsRBWo84951428;     RvSRVWKYfBXFBczsRBWo84951428 = RvSRVWKYfBXFBczsRBWo15630252;     RvSRVWKYfBXFBczsRBWo15630252 = RvSRVWKYfBXFBczsRBWo36145766;     RvSRVWKYfBXFBczsRBWo36145766 = RvSRVWKYfBXFBczsRBWo32421806;     RvSRVWKYfBXFBczsRBWo32421806 = RvSRVWKYfBXFBczsRBWo14275340;     RvSRVWKYfBXFBczsRBWo14275340 = RvSRVWKYfBXFBczsRBWo58873638;     RvSRVWKYfBXFBczsRBWo58873638 = RvSRVWKYfBXFBczsRBWo3737900;     RvSRVWKYfBXFBczsRBWo3737900 = RvSRVWKYfBXFBczsRBWo22689694;     RvSRVWKYfBXFBczsRBWo22689694 = RvSRVWKYfBXFBczsRBWo84598799;     RvSRVWKYfBXFBczsRBWo84598799 = RvSRVWKYfBXFBczsRBWo35955635;     RvSRVWKYfBXFBczsRBWo35955635 = RvSRVWKYfBXFBczsRBWo11554537;     RvSRVWKYfBXFBczsRBWo11554537 = RvSRVWKYfBXFBczsRBWo85581570;     RvSRVWKYfBXFBczsRBWo85581570 = RvSRVWKYfBXFBczsRBWo47377568;     RvSRVWKYfBXFBczsRBWo47377568 = RvSRVWKYfBXFBczsRBWo40842581;     RvSRVWKYfBXFBczsRBWo40842581 = RvSRVWKYfBXFBczsRBWo90513751;     RvSRVWKYfBXFBczsRBWo90513751 = RvSRVWKYfBXFBczsRBWo32114513;     RvSRVWKYfBXFBczsRBWo32114513 = RvSRVWKYfBXFBczsRBWo2249332;     RvSRVWKYfBXFBczsRBWo2249332 = RvSRVWKYfBXFBczsRBWo53759957;     RvSRVWKYfBXFBczsRBWo53759957 = RvSRVWKYfBXFBczsRBWo52016932;     RvSRVWKYfBXFBczsRBWo52016932 = RvSRVWKYfBXFBczsRBWo42939031;     RvSRVWKYfBXFBczsRBWo42939031 = RvSRVWKYfBXFBczsRBWo75751824;     RvSRVWKYfBXFBczsRBWo75751824 = RvSRVWKYfBXFBczsRBWo77807132;     RvSRVWKYfBXFBczsRBWo77807132 = RvSRVWKYfBXFBczsRBWo13113231;     RvSRVWKYfBXFBczsRBWo13113231 = RvSRVWKYfBXFBczsRBWo94801641;     RvSRVWKYfBXFBczsRBWo94801641 = RvSRVWKYfBXFBczsRBWo86018941;     RvSRVWKYfBXFBczsRBWo86018941 = RvSRVWKYfBXFBczsRBWo56410404;     RvSRVWKYfBXFBczsRBWo56410404 = RvSRVWKYfBXFBczsRBWo55689330;     RvSRVWKYfBXFBczsRBWo55689330 = RvSRVWKYfBXFBczsRBWo92496014;     RvSRVWKYfBXFBczsRBWo92496014 = RvSRVWKYfBXFBczsRBWo6400935;     RvSRVWKYfBXFBczsRBWo6400935 = RvSRVWKYfBXFBczsRBWo24493731;     RvSRVWKYfBXFBczsRBWo24493731 = RvSRVWKYfBXFBczsRBWo13803852;     RvSRVWKYfBXFBczsRBWo13803852 = RvSRVWKYfBXFBczsRBWo31315996;     RvSRVWKYfBXFBczsRBWo31315996 = RvSRVWKYfBXFBczsRBWo15502911;     RvSRVWKYfBXFBczsRBWo15502911 = RvSRVWKYfBXFBczsRBWo7140234;     RvSRVWKYfBXFBczsRBWo7140234 = RvSRVWKYfBXFBczsRBWo23909672;     RvSRVWKYfBXFBczsRBWo23909672 = RvSRVWKYfBXFBczsRBWo17505762;     RvSRVWKYfBXFBczsRBWo17505762 = RvSRVWKYfBXFBczsRBWo6717895;     RvSRVWKYfBXFBczsRBWo6717895 = RvSRVWKYfBXFBczsRBWo12691285;     RvSRVWKYfBXFBczsRBWo12691285 = RvSRVWKYfBXFBczsRBWo7677608;     RvSRVWKYfBXFBczsRBWo7677608 = RvSRVWKYfBXFBczsRBWo76011406;     RvSRVWKYfBXFBczsRBWo76011406 = RvSRVWKYfBXFBczsRBWo28486664;     RvSRVWKYfBXFBczsRBWo28486664 = RvSRVWKYfBXFBczsRBWo87537440;     RvSRVWKYfBXFBczsRBWo87537440 = RvSRVWKYfBXFBczsRBWo68271557;     RvSRVWKYfBXFBczsRBWo68271557 = RvSRVWKYfBXFBczsRBWo12601100;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FZyDcwLyeXDMnQmrEPGV37120616() {     double FgUbicBuYtoHavbukvNO38186376 = -205302753;    double FgUbicBuYtoHavbukvNO99595679 = -577742812;    double FgUbicBuYtoHavbukvNO48508093 = -822212350;    double FgUbicBuYtoHavbukvNO30396533 = -593973655;    double FgUbicBuYtoHavbukvNO73456218 = -10885296;    double FgUbicBuYtoHavbukvNO19236066 = -861591297;    double FgUbicBuYtoHavbukvNO60961360 = -615560713;    double FgUbicBuYtoHavbukvNO64600854 = -503014999;    double FgUbicBuYtoHavbukvNO38748750 = -938717375;    double FgUbicBuYtoHavbukvNO68246295 = -557879567;    double FgUbicBuYtoHavbukvNO5463314 = -270944681;    double FgUbicBuYtoHavbukvNO89519525 = -987563613;    double FgUbicBuYtoHavbukvNO42075692 = -685661110;    double FgUbicBuYtoHavbukvNO30081702 = -946837063;    double FgUbicBuYtoHavbukvNO28006268 = -433760585;    double FgUbicBuYtoHavbukvNO57424723 = -925759977;    double FgUbicBuYtoHavbukvNO23447881 = -51805495;    double FgUbicBuYtoHavbukvNO96094077 = -381249967;    double FgUbicBuYtoHavbukvNO8827142 = -559313118;    double FgUbicBuYtoHavbukvNO36874372 = -795263022;    double FgUbicBuYtoHavbukvNO87911242 = -800544032;    double FgUbicBuYtoHavbukvNO76666701 = -76063320;    double FgUbicBuYtoHavbukvNO89742756 = -161468152;    double FgUbicBuYtoHavbukvNO32391532 = -87983733;    double FgUbicBuYtoHavbukvNO23947491 = -124885459;    double FgUbicBuYtoHavbukvNO97457584 = -378188538;    double FgUbicBuYtoHavbukvNO52192892 = -862725252;    double FgUbicBuYtoHavbukvNO5263976 = -950299437;    double FgUbicBuYtoHavbukvNO98594304 = -251155333;    double FgUbicBuYtoHavbukvNO63004048 = -402003709;    double FgUbicBuYtoHavbukvNO48444854 = 57413746;    double FgUbicBuYtoHavbukvNO43720893 = -14819793;    double FgUbicBuYtoHavbukvNO39433286 = -477462918;    double FgUbicBuYtoHavbukvNO7527450 = -951253649;    double FgUbicBuYtoHavbukvNO54345404 = -676920726;    double FgUbicBuYtoHavbukvNO29151662 = -52511629;    double FgUbicBuYtoHavbukvNO58506485 = -514552772;    double FgUbicBuYtoHavbukvNO8708700 = -641549623;    double FgUbicBuYtoHavbukvNO83787239 = -781792688;    double FgUbicBuYtoHavbukvNO53896763 = -745903868;    double FgUbicBuYtoHavbukvNO69104228 = -271360406;    double FgUbicBuYtoHavbukvNO78894635 = 82725658;    double FgUbicBuYtoHavbukvNO81424783 = -334270679;    double FgUbicBuYtoHavbukvNO81020413 = -800621606;    double FgUbicBuYtoHavbukvNO44851233 = -707972302;    double FgUbicBuYtoHavbukvNO22331443 = -800150266;    double FgUbicBuYtoHavbukvNO72935260 = -848049732;    double FgUbicBuYtoHavbukvNO91898812 = 71693301;    double FgUbicBuYtoHavbukvNO35766092 = -380564759;    double FgUbicBuYtoHavbukvNO88837757 = -118850528;    double FgUbicBuYtoHavbukvNO27321790 = -700719224;    double FgUbicBuYtoHavbukvNO47255864 = -376981102;    double FgUbicBuYtoHavbukvNO110834 = -278205988;    double FgUbicBuYtoHavbukvNO73341291 = -531328293;    double FgUbicBuYtoHavbukvNO49209374 = -285259035;    double FgUbicBuYtoHavbukvNO61519675 = -29239433;    double FgUbicBuYtoHavbukvNO9852924 = -316274660;    double FgUbicBuYtoHavbukvNO16116562 = -634228617;    double FgUbicBuYtoHavbukvNO6449042 = -369088196;    double FgUbicBuYtoHavbukvNO75998634 = -632696758;    double FgUbicBuYtoHavbukvNO67043173 = -998866045;    double FgUbicBuYtoHavbukvNO55697384 = -665261276;    double FgUbicBuYtoHavbukvNO66006550 = -151859666;    double FgUbicBuYtoHavbukvNO75744701 = -436713666;    double FgUbicBuYtoHavbukvNO19801442 = -515293313;    double FgUbicBuYtoHavbukvNO61742421 = -156124889;    double FgUbicBuYtoHavbukvNO50086239 = -410100696;    double FgUbicBuYtoHavbukvNO34548243 = -734407461;    double FgUbicBuYtoHavbukvNO75736298 = -169916337;    double FgUbicBuYtoHavbukvNO98854605 = -281248957;    double FgUbicBuYtoHavbukvNO98918238 = -311207205;    double FgUbicBuYtoHavbukvNO14739181 = -410255872;    double FgUbicBuYtoHavbukvNO12306839 = -599457279;    double FgUbicBuYtoHavbukvNO54930378 = -813409251;    double FgUbicBuYtoHavbukvNO67770143 = -423902617;    double FgUbicBuYtoHavbukvNO9016608 = -783269691;    double FgUbicBuYtoHavbukvNO95241918 = -741792642;    double FgUbicBuYtoHavbukvNO8722343 = -360846546;    double FgUbicBuYtoHavbukvNO87540298 = -380011431;    double FgUbicBuYtoHavbukvNO1616048 = -324735194;    double FgUbicBuYtoHavbukvNO24522324 = -530138807;    double FgUbicBuYtoHavbukvNO60294080 = -834418554;    double FgUbicBuYtoHavbukvNO69497884 = -469734679;    double FgUbicBuYtoHavbukvNO9756547 = -32304806;    double FgUbicBuYtoHavbukvNO35682258 = -701284486;    double FgUbicBuYtoHavbukvNO1188990 = -565605152;    double FgUbicBuYtoHavbukvNO43610060 = -736613806;    double FgUbicBuYtoHavbukvNO66091995 = -946134626;    double FgUbicBuYtoHavbukvNO58318075 = -565994615;    double FgUbicBuYtoHavbukvNO92825728 = -547681294;    double FgUbicBuYtoHavbukvNO19298739 = -736236969;    double FgUbicBuYtoHavbukvNO42389924 = -880324156;    double FgUbicBuYtoHavbukvNO2259659 = -172461427;    double FgUbicBuYtoHavbukvNO7788606 = -49095930;    double FgUbicBuYtoHavbukvNO86853590 = -747037823;    double FgUbicBuYtoHavbukvNO13406844 = -606099131;    double FgUbicBuYtoHavbukvNO12888085 = -765414676;    double FgUbicBuYtoHavbukvNO5680082 = -897557014;    double FgUbicBuYtoHavbukvNO61218971 = -185328294;    double FgUbicBuYtoHavbukvNO83108812 = -205302753;     FgUbicBuYtoHavbukvNO38186376 = FgUbicBuYtoHavbukvNO99595679;     FgUbicBuYtoHavbukvNO99595679 = FgUbicBuYtoHavbukvNO48508093;     FgUbicBuYtoHavbukvNO48508093 = FgUbicBuYtoHavbukvNO30396533;     FgUbicBuYtoHavbukvNO30396533 = FgUbicBuYtoHavbukvNO73456218;     FgUbicBuYtoHavbukvNO73456218 = FgUbicBuYtoHavbukvNO19236066;     FgUbicBuYtoHavbukvNO19236066 = FgUbicBuYtoHavbukvNO60961360;     FgUbicBuYtoHavbukvNO60961360 = FgUbicBuYtoHavbukvNO64600854;     FgUbicBuYtoHavbukvNO64600854 = FgUbicBuYtoHavbukvNO38748750;     FgUbicBuYtoHavbukvNO38748750 = FgUbicBuYtoHavbukvNO68246295;     FgUbicBuYtoHavbukvNO68246295 = FgUbicBuYtoHavbukvNO5463314;     FgUbicBuYtoHavbukvNO5463314 = FgUbicBuYtoHavbukvNO89519525;     FgUbicBuYtoHavbukvNO89519525 = FgUbicBuYtoHavbukvNO42075692;     FgUbicBuYtoHavbukvNO42075692 = FgUbicBuYtoHavbukvNO30081702;     FgUbicBuYtoHavbukvNO30081702 = FgUbicBuYtoHavbukvNO28006268;     FgUbicBuYtoHavbukvNO28006268 = FgUbicBuYtoHavbukvNO57424723;     FgUbicBuYtoHavbukvNO57424723 = FgUbicBuYtoHavbukvNO23447881;     FgUbicBuYtoHavbukvNO23447881 = FgUbicBuYtoHavbukvNO96094077;     FgUbicBuYtoHavbukvNO96094077 = FgUbicBuYtoHavbukvNO8827142;     FgUbicBuYtoHavbukvNO8827142 = FgUbicBuYtoHavbukvNO36874372;     FgUbicBuYtoHavbukvNO36874372 = FgUbicBuYtoHavbukvNO87911242;     FgUbicBuYtoHavbukvNO87911242 = FgUbicBuYtoHavbukvNO76666701;     FgUbicBuYtoHavbukvNO76666701 = FgUbicBuYtoHavbukvNO89742756;     FgUbicBuYtoHavbukvNO89742756 = FgUbicBuYtoHavbukvNO32391532;     FgUbicBuYtoHavbukvNO32391532 = FgUbicBuYtoHavbukvNO23947491;     FgUbicBuYtoHavbukvNO23947491 = FgUbicBuYtoHavbukvNO97457584;     FgUbicBuYtoHavbukvNO97457584 = FgUbicBuYtoHavbukvNO52192892;     FgUbicBuYtoHavbukvNO52192892 = FgUbicBuYtoHavbukvNO5263976;     FgUbicBuYtoHavbukvNO5263976 = FgUbicBuYtoHavbukvNO98594304;     FgUbicBuYtoHavbukvNO98594304 = FgUbicBuYtoHavbukvNO63004048;     FgUbicBuYtoHavbukvNO63004048 = FgUbicBuYtoHavbukvNO48444854;     FgUbicBuYtoHavbukvNO48444854 = FgUbicBuYtoHavbukvNO43720893;     FgUbicBuYtoHavbukvNO43720893 = FgUbicBuYtoHavbukvNO39433286;     FgUbicBuYtoHavbukvNO39433286 = FgUbicBuYtoHavbukvNO7527450;     FgUbicBuYtoHavbukvNO7527450 = FgUbicBuYtoHavbukvNO54345404;     FgUbicBuYtoHavbukvNO54345404 = FgUbicBuYtoHavbukvNO29151662;     FgUbicBuYtoHavbukvNO29151662 = FgUbicBuYtoHavbukvNO58506485;     FgUbicBuYtoHavbukvNO58506485 = FgUbicBuYtoHavbukvNO8708700;     FgUbicBuYtoHavbukvNO8708700 = FgUbicBuYtoHavbukvNO83787239;     FgUbicBuYtoHavbukvNO83787239 = FgUbicBuYtoHavbukvNO53896763;     FgUbicBuYtoHavbukvNO53896763 = FgUbicBuYtoHavbukvNO69104228;     FgUbicBuYtoHavbukvNO69104228 = FgUbicBuYtoHavbukvNO78894635;     FgUbicBuYtoHavbukvNO78894635 = FgUbicBuYtoHavbukvNO81424783;     FgUbicBuYtoHavbukvNO81424783 = FgUbicBuYtoHavbukvNO81020413;     FgUbicBuYtoHavbukvNO81020413 = FgUbicBuYtoHavbukvNO44851233;     FgUbicBuYtoHavbukvNO44851233 = FgUbicBuYtoHavbukvNO22331443;     FgUbicBuYtoHavbukvNO22331443 = FgUbicBuYtoHavbukvNO72935260;     FgUbicBuYtoHavbukvNO72935260 = FgUbicBuYtoHavbukvNO91898812;     FgUbicBuYtoHavbukvNO91898812 = FgUbicBuYtoHavbukvNO35766092;     FgUbicBuYtoHavbukvNO35766092 = FgUbicBuYtoHavbukvNO88837757;     FgUbicBuYtoHavbukvNO88837757 = FgUbicBuYtoHavbukvNO27321790;     FgUbicBuYtoHavbukvNO27321790 = FgUbicBuYtoHavbukvNO47255864;     FgUbicBuYtoHavbukvNO47255864 = FgUbicBuYtoHavbukvNO110834;     FgUbicBuYtoHavbukvNO110834 = FgUbicBuYtoHavbukvNO73341291;     FgUbicBuYtoHavbukvNO73341291 = FgUbicBuYtoHavbukvNO49209374;     FgUbicBuYtoHavbukvNO49209374 = FgUbicBuYtoHavbukvNO61519675;     FgUbicBuYtoHavbukvNO61519675 = FgUbicBuYtoHavbukvNO9852924;     FgUbicBuYtoHavbukvNO9852924 = FgUbicBuYtoHavbukvNO16116562;     FgUbicBuYtoHavbukvNO16116562 = FgUbicBuYtoHavbukvNO6449042;     FgUbicBuYtoHavbukvNO6449042 = FgUbicBuYtoHavbukvNO75998634;     FgUbicBuYtoHavbukvNO75998634 = FgUbicBuYtoHavbukvNO67043173;     FgUbicBuYtoHavbukvNO67043173 = FgUbicBuYtoHavbukvNO55697384;     FgUbicBuYtoHavbukvNO55697384 = FgUbicBuYtoHavbukvNO66006550;     FgUbicBuYtoHavbukvNO66006550 = FgUbicBuYtoHavbukvNO75744701;     FgUbicBuYtoHavbukvNO75744701 = FgUbicBuYtoHavbukvNO19801442;     FgUbicBuYtoHavbukvNO19801442 = FgUbicBuYtoHavbukvNO61742421;     FgUbicBuYtoHavbukvNO61742421 = FgUbicBuYtoHavbukvNO50086239;     FgUbicBuYtoHavbukvNO50086239 = FgUbicBuYtoHavbukvNO34548243;     FgUbicBuYtoHavbukvNO34548243 = FgUbicBuYtoHavbukvNO75736298;     FgUbicBuYtoHavbukvNO75736298 = FgUbicBuYtoHavbukvNO98854605;     FgUbicBuYtoHavbukvNO98854605 = FgUbicBuYtoHavbukvNO98918238;     FgUbicBuYtoHavbukvNO98918238 = FgUbicBuYtoHavbukvNO14739181;     FgUbicBuYtoHavbukvNO14739181 = FgUbicBuYtoHavbukvNO12306839;     FgUbicBuYtoHavbukvNO12306839 = FgUbicBuYtoHavbukvNO54930378;     FgUbicBuYtoHavbukvNO54930378 = FgUbicBuYtoHavbukvNO67770143;     FgUbicBuYtoHavbukvNO67770143 = FgUbicBuYtoHavbukvNO9016608;     FgUbicBuYtoHavbukvNO9016608 = FgUbicBuYtoHavbukvNO95241918;     FgUbicBuYtoHavbukvNO95241918 = FgUbicBuYtoHavbukvNO8722343;     FgUbicBuYtoHavbukvNO8722343 = FgUbicBuYtoHavbukvNO87540298;     FgUbicBuYtoHavbukvNO87540298 = FgUbicBuYtoHavbukvNO1616048;     FgUbicBuYtoHavbukvNO1616048 = FgUbicBuYtoHavbukvNO24522324;     FgUbicBuYtoHavbukvNO24522324 = FgUbicBuYtoHavbukvNO60294080;     FgUbicBuYtoHavbukvNO60294080 = FgUbicBuYtoHavbukvNO69497884;     FgUbicBuYtoHavbukvNO69497884 = FgUbicBuYtoHavbukvNO9756547;     FgUbicBuYtoHavbukvNO9756547 = FgUbicBuYtoHavbukvNO35682258;     FgUbicBuYtoHavbukvNO35682258 = FgUbicBuYtoHavbukvNO1188990;     FgUbicBuYtoHavbukvNO1188990 = FgUbicBuYtoHavbukvNO43610060;     FgUbicBuYtoHavbukvNO43610060 = FgUbicBuYtoHavbukvNO66091995;     FgUbicBuYtoHavbukvNO66091995 = FgUbicBuYtoHavbukvNO58318075;     FgUbicBuYtoHavbukvNO58318075 = FgUbicBuYtoHavbukvNO92825728;     FgUbicBuYtoHavbukvNO92825728 = FgUbicBuYtoHavbukvNO19298739;     FgUbicBuYtoHavbukvNO19298739 = FgUbicBuYtoHavbukvNO42389924;     FgUbicBuYtoHavbukvNO42389924 = FgUbicBuYtoHavbukvNO2259659;     FgUbicBuYtoHavbukvNO2259659 = FgUbicBuYtoHavbukvNO7788606;     FgUbicBuYtoHavbukvNO7788606 = FgUbicBuYtoHavbukvNO86853590;     FgUbicBuYtoHavbukvNO86853590 = FgUbicBuYtoHavbukvNO13406844;     FgUbicBuYtoHavbukvNO13406844 = FgUbicBuYtoHavbukvNO12888085;     FgUbicBuYtoHavbukvNO12888085 = FgUbicBuYtoHavbukvNO5680082;     FgUbicBuYtoHavbukvNO5680082 = FgUbicBuYtoHavbukvNO61218971;     FgUbicBuYtoHavbukvNO61218971 = FgUbicBuYtoHavbukvNO83108812;     FgUbicBuYtoHavbukvNO83108812 = FgUbicBuYtoHavbukvNO38186376;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void eAOJjMJvgnTyjakOpVpI64714872() {     double KuTktsBXErNjBqnjzkll63771653 = -730335853;    double KuTktsBXErNjBqnjzkll95992822 = -833220589;    double KuTktsBXErNjBqnjzkll81387268 = -624673886;    double KuTktsBXErNjBqnjzkll76118462 = -601296549;    double KuTktsBXErNjBqnjzkll42798223 = -57807583;    double KuTktsBXErNjBqnjzkll36245342 = 83427699;    double KuTktsBXErNjBqnjzkll66282204 = -97831919;    double KuTktsBXErNjBqnjzkll79610978 = -883696878;    double KuTktsBXErNjBqnjzkll64078042 = -334941419;    double KuTktsBXErNjBqnjzkll87404678 = -892441090;    double KuTktsBXErNjBqnjzkll19796868 = -581372568;    double KuTktsBXErNjBqnjzkll24837181 = -331272520;    double KuTktsBXErNjBqnjzkll18656470 = -449909475;    double KuTktsBXErNjBqnjzkll21641419 = -921361356;    double KuTktsBXErNjBqnjzkll84275155 = -322306543;    double KuTktsBXErNjBqnjzkll65145766 = -261951454;    double KuTktsBXErNjBqnjzkll23245997 = -840125289;    double KuTktsBXErNjBqnjzkll17209982 = -897527328;    double KuTktsBXErNjBqnjzkll70291863 = -242286975;    double KuTktsBXErNjBqnjzkll98035838 = -769702398;    double KuTktsBXErNjBqnjzkll88920507 = -162243813;    double KuTktsBXErNjBqnjzkll78169808 = -495796250;    double KuTktsBXErNjBqnjzkll93506553 = -205560258;    double KuTktsBXErNjBqnjzkll34105574 = 53167288;    double KuTktsBXErNjBqnjzkll78850630 = -792381602;    double KuTktsBXErNjBqnjzkll26946721 = 57360921;    double KuTktsBXErNjBqnjzkll34580802 = -428538387;    double KuTktsBXErNjBqnjzkll69162776 = -79961670;    double KuTktsBXErNjBqnjzkll6471517 = -487449184;    double KuTktsBXErNjBqnjzkll16326539 = -310913271;    double KuTktsBXErNjBqnjzkll70491488 = -821233285;    double KuTktsBXErNjBqnjzkll80910825 = -288615627;    double KuTktsBXErNjBqnjzkll60620338 = -925284055;    double KuTktsBXErNjBqnjzkll61114520 = 92601379;    double KuTktsBXErNjBqnjzkll55750392 = -278356638;    double KuTktsBXErNjBqnjzkll33943512 = -570106345;    double KuTktsBXErNjBqnjzkll8151871 = -995064717;    double KuTktsBXErNjBqnjzkll84281386 = -490264448;    double KuTktsBXErNjBqnjzkll24710818 = -140131010;    double KuTktsBXErNjBqnjzkll62680437 = -115874187;    double KuTktsBXErNjBqnjzkll16255510 = -846420596;    double KuTktsBXErNjBqnjzkll22904225 = -426312846;    double KuTktsBXErNjBqnjzkll30625003 = -362877060;    double KuTktsBXErNjBqnjzkll51813692 = -169037205;    double KuTktsBXErNjBqnjzkll36832109 = -646993419;    double KuTktsBXErNjBqnjzkll88731765 = -222697803;    double KuTktsBXErNjBqnjzkll72703714 = -880518592;    double KuTktsBXErNjBqnjzkll11584 = -121489545;    double KuTktsBXErNjBqnjzkll86577411 = -480240985;    double KuTktsBXErNjBqnjzkll42647752 = 33126319;    double KuTktsBXErNjBqnjzkll37458038 = -949377826;    double KuTktsBXErNjBqnjzkll74514444 = -555060282;    double KuTktsBXErNjBqnjzkll18184437 = -846104537;    double KuTktsBXErNjBqnjzkll42240200 = -846392074;    double KuTktsBXErNjBqnjzkll75794365 = -760349452;    double KuTktsBXErNjBqnjzkll85601845 = -134539604;    double KuTktsBXErNjBqnjzkll2486269 = -527660332;    double KuTktsBXErNjBqnjzkll47281695 = -577841174;    double KuTktsBXErNjBqnjzkll97267832 = -808914947;    double KuTktsBXErNjBqnjzkll15851503 = -15168504;    double KuTktsBXErNjBqnjzkll1664541 = -488033914;    double KuTktsBXErNjBqnjzkll97119428 = 82129750;    double KuTktsBXErNjBqnjzkll73139462 = -296247695;    double KuTktsBXErNjBqnjzkll47751504 = 75971851;    double KuTktsBXErNjBqnjzkll16913190 = 28792194;    double KuTktsBXErNjBqnjzkll38886042 = -192756941;    double KuTktsBXErNjBqnjzkll64216843 = -405988465;    double KuTktsBXErNjBqnjzkll57541950 = -442510855;    double KuTktsBXErNjBqnjzkll65891027 = -543004719;    double KuTktsBXErNjBqnjzkll50331643 = -752200199;    double KuTktsBXErNjBqnjzkll56993896 = -266886737;    double KuTktsBXErNjBqnjzkll38964610 = -249860841;    double KuTktsBXErNjBqnjzkll92499164 = -657396318;    double KuTktsBXErNjBqnjzkll7611426 = -26412789;    double KuTktsBXErNjBqnjzkll81780328 = -923281803;    double KuTktsBXErNjBqnjzkll66016282 = -735930968;    double KuTktsBXErNjBqnjzkll47544806 = -32919190;    double KuTktsBXErNjBqnjzkll41692861 = 63476947;    double KuTktsBXErNjBqnjzkll97273465 = -299839294;    double KuTktsBXErNjBqnjzkll90118864 = -469683799;    double KuTktsBXErNjBqnjzkll54243006 = -62120488;    double KuTktsBXErNjBqnjzkll34569218 = -207048842;    double KuTktsBXErNjBqnjzkll82585364 = -599720685;    double KuTktsBXErNjBqnjzkll63823764 = -420575503;    double KuTktsBXErNjBqnjzkll78868501 = -361535446;    double KuTktsBXErNjBqnjzkll95977044 = -166173004;    double KuTktsBXErNjBqnjzkll62726389 = -442511091;    double KuTktsBXErNjBqnjzkll18380139 = 21108019;    double KuTktsBXErNjBqnjzkll85320155 = -147049169;    double KuTktsBXErNjBqnjzkll70148546 = -43817034;    double KuTktsBXErNjBqnjzkll31457244 = 57553987;    double KuTktsBXErNjBqnjzkll60870176 = -317223544;    double KuTktsBXErNjBqnjzkll87013554 = -681349501;    double KuTktsBXErNjBqnjzkll8859316 = -24962507;    double KuTktsBXErNjBqnjzkll61015897 = -627840273;    double KuTktsBXErNjBqnjzkll19136081 = -828550346;    double KuTktsBXErNjBqnjzkll49764763 = -30065152;    double KuTktsBXErNjBqnjzkll82873499 = -338848912;    double KuTktsBXErNjBqnjzkll34900503 = -97829400;    double KuTktsBXErNjBqnjzkll97946066 = -730335853;     KuTktsBXErNjBqnjzkll63771653 = KuTktsBXErNjBqnjzkll95992822;     KuTktsBXErNjBqnjzkll95992822 = KuTktsBXErNjBqnjzkll81387268;     KuTktsBXErNjBqnjzkll81387268 = KuTktsBXErNjBqnjzkll76118462;     KuTktsBXErNjBqnjzkll76118462 = KuTktsBXErNjBqnjzkll42798223;     KuTktsBXErNjBqnjzkll42798223 = KuTktsBXErNjBqnjzkll36245342;     KuTktsBXErNjBqnjzkll36245342 = KuTktsBXErNjBqnjzkll66282204;     KuTktsBXErNjBqnjzkll66282204 = KuTktsBXErNjBqnjzkll79610978;     KuTktsBXErNjBqnjzkll79610978 = KuTktsBXErNjBqnjzkll64078042;     KuTktsBXErNjBqnjzkll64078042 = KuTktsBXErNjBqnjzkll87404678;     KuTktsBXErNjBqnjzkll87404678 = KuTktsBXErNjBqnjzkll19796868;     KuTktsBXErNjBqnjzkll19796868 = KuTktsBXErNjBqnjzkll24837181;     KuTktsBXErNjBqnjzkll24837181 = KuTktsBXErNjBqnjzkll18656470;     KuTktsBXErNjBqnjzkll18656470 = KuTktsBXErNjBqnjzkll21641419;     KuTktsBXErNjBqnjzkll21641419 = KuTktsBXErNjBqnjzkll84275155;     KuTktsBXErNjBqnjzkll84275155 = KuTktsBXErNjBqnjzkll65145766;     KuTktsBXErNjBqnjzkll65145766 = KuTktsBXErNjBqnjzkll23245997;     KuTktsBXErNjBqnjzkll23245997 = KuTktsBXErNjBqnjzkll17209982;     KuTktsBXErNjBqnjzkll17209982 = KuTktsBXErNjBqnjzkll70291863;     KuTktsBXErNjBqnjzkll70291863 = KuTktsBXErNjBqnjzkll98035838;     KuTktsBXErNjBqnjzkll98035838 = KuTktsBXErNjBqnjzkll88920507;     KuTktsBXErNjBqnjzkll88920507 = KuTktsBXErNjBqnjzkll78169808;     KuTktsBXErNjBqnjzkll78169808 = KuTktsBXErNjBqnjzkll93506553;     KuTktsBXErNjBqnjzkll93506553 = KuTktsBXErNjBqnjzkll34105574;     KuTktsBXErNjBqnjzkll34105574 = KuTktsBXErNjBqnjzkll78850630;     KuTktsBXErNjBqnjzkll78850630 = KuTktsBXErNjBqnjzkll26946721;     KuTktsBXErNjBqnjzkll26946721 = KuTktsBXErNjBqnjzkll34580802;     KuTktsBXErNjBqnjzkll34580802 = KuTktsBXErNjBqnjzkll69162776;     KuTktsBXErNjBqnjzkll69162776 = KuTktsBXErNjBqnjzkll6471517;     KuTktsBXErNjBqnjzkll6471517 = KuTktsBXErNjBqnjzkll16326539;     KuTktsBXErNjBqnjzkll16326539 = KuTktsBXErNjBqnjzkll70491488;     KuTktsBXErNjBqnjzkll70491488 = KuTktsBXErNjBqnjzkll80910825;     KuTktsBXErNjBqnjzkll80910825 = KuTktsBXErNjBqnjzkll60620338;     KuTktsBXErNjBqnjzkll60620338 = KuTktsBXErNjBqnjzkll61114520;     KuTktsBXErNjBqnjzkll61114520 = KuTktsBXErNjBqnjzkll55750392;     KuTktsBXErNjBqnjzkll55750392 = KuTktsBXErNjBqnjzkll33943512;     KuTktsBXErNjBqnjzkll33943512 = KuTktsBXErNjBqnjzkll8151871;     KuTktsBXErNjBqnjzkll8151871 = KuTktsBXErNjBqnjzkll84281386;     KuTktsBXErNjBqnjzkll84281386 = KuTktsBXErNjBqnjzkll24710818;     KuTktsBXErNjBqnjzkll24710818 = KuTktsBXErNjBqnjzkll62680437;     KuTktsBXErNjBqnjzkll62680437 = KuTktsBXErNjBqnjzkll16255510;     KuTktsBXErNjBqnjzkll16255510 = KuTktsBXErNjBqnjzkll22904225;     KuTktsBXErNjBqnjzkll22904225 = KuTktsBXErNjBqnjzkll30625003;     KuTktsBXErNjBqnjzkll30625003 = KuTktsBXErNjBqnjzkll51813692;     KuTktsBXErNjBqnjzkll51813692 = KuTktsBXErNjBqnjzkll36832109;     KuTktsBXErNjBqnjzkll36832109 = KuTktsBXErNjBqnjzkll88731765;     KuTktsBXErNjBqnjzkll88731765 = KuTktsBXErNjBqnjzkll72703714;     KuTktsBXErNjBqnjzkll72703714 = KuTktsBXErNjBqnjzkll11584;     KuTktsBXErNjBqnjzkll11584 = KuTktsBXErNjBqnjzkll86577411;     KuTktsBXErNjBqnjzkll86577411 = KuTktsBXErNjBqnjzkll42647752;     KuTktsBXErNjBqnjzkll42647752 = KuTktsBXErNjBqnjzkll37458038;     KuTktsBXErNjBqnjzkll37458038 = KuTktsBXErNjBqnjzkll74514444;     KuTktsBXErNjBqnjzkll74514444 = KuTktsBXErNjBqnjzkll18184437;     KuTktsBXErNjBqnjzkll18184437 = KuTktsBXErNjBqnjzkll42240200;     KuTktsBXErNjBqnjzkll42240200 = KuTktsBXErNjBqnjzkll75794365;     KuTktsBXErNjBqnjzkll75794365 = KuTktsBXErNjBqnjzkll85601845;     KuTktsBXErNjBqnjzkll85601845 = KuTktsBXErNjBqnjzkll2486269;     KuTktsBXErNjBqnjzkll2486269 = KuTktsBXErNjBqnjzkll47281695;     KuTktsBXErNjBqnjzkll47281695 = KuTktsBXErNjBqnjzkll97267832;     KuTktsBXErNjBqnjzkll97267832 = KuTktsBXErNjBqnjzkll15851503;     KuTktsBXErNjBqnjzkll15851503 = KuTktsBXErNjBqnjzkll1664541;     KuTktsBXErNjBqnjzkll1664541 = KuTktsBXErNjBqnjzkll97119428;     KuTktsBXErNjBqnjzkll97119428 = KuTktsBXErNjBqnjzkll73139462;     KuTktsBXErNjBqnjzkll73139462 = KuTktsBXErNjBqnjzkll47751504;     KuTktsBXErNjBqnjzkll47751504 = KuTktsBXErNjBqnjzkll16913190;     KuTktsBXErNjBqnjzkll16913190 = KuTktsBXErNjBqnjzkll38886042;     KuTktsBXErNjBqnjzkll38886042 = KuTktsBXErNjBqnjzkll64216843;     KuTktsBXErNjBqnjzkll64216843 = KuTktsBXErNjBqnjzkll57541950;     KuTktsBXErNjBqnjzkll57541950 = KuTktsBXErNjBqnjzkll65891027;     KuTktsBXErNjBqnjzkll65891027 = KuTktsBXErNjBqnjzkll50331643;     KuTktsBXErNjBqnjzkll50331643 = KuTktsBXErNjBqnjzkll56993896;     KuTktsBXErNjBqnjzkll56993896 = KuTktsBXErNjBqnjzkll38964610;     KuTktsBXErNjBqnjzkll38964610 = KuTktsBXErNjBqnjzkll92499164;     KuTktsBXErNjBqnjzkll92499164 = KuTktsBXErNjBqnjzkll7611426;     KuTktsBXErNjBqnjzkll7611426 = KuTktsBXErNjBqnjzkll81780328;     KuTktsBXErNjBqnjzkll81780328 = KuTktsBXErNjBqnjzkll66016282;     KuTktsBXErNjBqnjzkll66016282 = KuTktsBXErNjBqnjzkll47544806;     KuTktsBXErNjBqnjzkll47544806 = KuTktsBXErNjBqnjzkll41692861;     KuTktsBXErNjBqnjzkll41692861 = KuTktsBXErNjBqnjzkll97273465;     KuTktsBXErNjBqnjzkll97273465 = KuTktsBXErNjBqnjzkll90118864;     KuTktsBXErNjBqnjzkll90118864 = KuTktsBXErNjBqnjzkll54243006;     KuTktsBXErNjBqnjzkll54243006 = KuTktsBXErNjBqnjzkll34569218;     KuTktsBXErNjBqnjzkll34569218 = KuTktsBXErNjBqnjzkll82585364;     KuTktsBXErNjBqnjzkll82585364 = KuTktsBXErNjBqnjzkll63823764;     KuTktsBXErNjBqnjzkll63823764 = KuTktsBXErNjBqnjzkll78868501;     KuTktsBXErNjBqnjzkll78868501 = KuTktsBXErNjBqnjzkll95977044;     KuTktsBXErNjBqnjzkll95977044 = KuTktsBXErNjBqnjzkll62726389;     KuTktsBXErNjBqnjzkll62726389 = KuTktsBXErNjBqnjzkll18380139;     KuTktsBXErNjBqnjzkll18380139 = KuTktsBXErNjBqnjzkll85320155;     KuTktsBXErNjBqnjzkll85320155 = KuTktsBXErNjBqnjzkll70148546;     KuTktsBXErNjBqnjzkll70148546 = KuTktsBXErNjBqnjzkll31457244;     KuTktsBXErNjBqnjzkll31457244 = KuTktsBXErNjBqnjzkll60870176;     KuTktsBXErNjBqnjzkll60870176 = KuTktsBXErNjBqnjzkll87013554;     KuTktsBXErNjBqnjzkll87013554 = KuTktsBXErNjBqnjzkll8859316;     KuTktsBXErNjBqnjzkll8859316 = KuTktsBXErNjBqnjzkll61015897;     KuTktsBXErNjBqnjzkll61015897 = KuTktsBXErNjBqnjzkll19136081;     KuTktsBXErNjBqnjzkll19136081 = KuTktsBXErNjBqnjzkll49764763;     KuTktsBXErNjBqnjzkll49764763 = KuTktsBXErNjBqnjzkll82873499;     KuTktsBXErNjBqnjzkll82873499 = KuTktsBXErNjBqnjzkll34900503;     KuTktsBXErNjBqnjzkll34900503 = KuTktsBXErNjBqnjzkll97946066;     KuTktsBXErNjBqnjzkll97946066 = KuTktsBXErNjBqnjzkll63771653;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OsyEmEuoYHOoaBXoJMQq92309129() {     double WdPgDAVGkNEjfuecuHXA89356930 = -155368954;    double WdPgDAVGkNEjfuecuHXA92389965 = 11301634;    double WdPgDAVGkNEjfuecuHXA14266445 = -427135422;    double WdPgDAVGkNEjfuecuHXA21840392 = -608619442;    double WdPgDAVGkNEjfuecuHXA12140228 = -104729870;    double WdPgDAVGkNEjfuecuHXA53254618 = -71553304;    double WdPgDAVGkNEjfuecuHXA71603049 = -680103126;    double WdPgDAVGkNEjfuecuHXA94621102 = -164378757;    double WdPgDAVGkNEjfuecuHXA89407335 = -831165464;    double WdPgDAVGkNEjfuecuHXA6563061 = -127002614;    double WdPgDAVGkNEjfuecuHXA34130422 = -891800455;    double WdPgDAVGkNEjfuecuHXA60154837 = -774981426;    double WdPgDAVGkNEjfuecuHXA95237247 = -214157841;    double WdPgDAVGkNEjfuecuHXA13201136 = -895885650;    double WdPgDAVGkNEjfuecuHXA40544044 = -210852501;    double WdPgDAVGkNEjfuecuHXA72866809 = -698142931;    double WdPgDAVGkNEjfuecuHXA23044113 = -528445083;    double WdPgDAVGkNEjfuecuHXA38325886 = -313804688;    double WdPgDAVGkNEjfuecuHXA31756584 = 74739167;    double WdPgDAVGkNEjfuecuHXA59197305 = -744141773;    double WdPgDAVGkNEjfuecuHXA89929772 = -623943594;    double WdPgDAVGkNEjfuecuHXA79672915 = -915529179;    double WdPgDAVGkNEjfuecuHXA97270351 = -249652363;    double WdPgDAVGkNEjfuecuHXA35819617 = -905681692;    double WdPgDAVGkNEjfuecuHXA33753769 = -359877744;    double WdPgDAVGkNEjfuecuHXA56435857 = -607089621;    double WdPgDAVGkNEjfuecuHXA16968711 = 5648478;    double WdPgDAVGkNEjfuecuHXA33061576 = -309623903;    double WdPgDAVGkNEjfuecuHXA14348729 = -723743034;    double WdPgDAVGkNEjfuecuHXA69649029 = -219822833;    double WdPgDAVGkNEjfuecuHXA92538122 = -599880316;    double WdPgDAVGkNEjfuecuHXA18100758 = -562411461;    double WdPgDAVGkNEjfuecuHXA81807390 = -273105192;    double WdPgDAVGkNEjfuecuHXA14701592 = 36456407;    double WdPgDAVGkNEjfuecuHXA57155380 = -979792549;    double WdPgDAVGkNEjfuecuHXA38735363 = 12298939;    double WdPgDAVGkNEjfuecuHXA57797256 = -375576662;    double WdPgDAVGkNEjfuecuHXA59854073 = -338979273;    double WdPgDAVGkNEjfuecuHXA65634396 = -598469332;    double WdPgDAVGkNEjfuecuHXA71464111 = -585844506;    double WdPgDAVGkNEjfuecuHXA63406790 = -321480786;    double WdPgDAVGkNEjfuecuHXA66913815 = -935351350;    double WdPgDAVGkNEjfuecuHXA79825222 = -391483442;    double WdPgDAVGkNEjfuecuHXA22606972 = -637452804;    double WdPgDAVGkNEjfuecuHXA28812985 = -586014537;    double WdPgDAVGkNEjfuecuHXA55132087 = -745245341;    double WdPgDAVGkNEjfuecuHXA72472168 = -912987452;    double WdPgDAVGkNEjfuecuHXA8124355 = -314672392;    double WdPgDAVGkNEjfuecuHXA37388732 = -579917212;    double WdPgDAVGkNEjfuecuHXA96457747 = -914896834;    double WdPgDAVGkNEjfuecuHXA47594285 = -98036428;    double WdPgDAVGkNEjfuecuHXA1773024 = -733139461;    double WdPgDAVGkNEjfuecuHXA36258041 = -314003085;    double WdPgDAVGkNEjfuecuHXA11139109 = -61455855;    double WdPgDAVGkNEjfuecuHXA2379357 = -135439869;    double WdPgDAVGkNEjfuecuHXA9684016 = -239839775;    double WdPgDAVGkNEjfuecuHXA95119614 = -739046003;    double WdPgDAVGkNEjfuecuHXA78446827 = -521453730;    double WdPgDAVGkNEjfuecuHXA88086622 = -148741698;    double WdPgDAVGkNEjfuecuHXA55704370 = -497640249;    double WdPgDAVGkNEjfuecuHXA36285908 = 22798217;    double WdPgDAVGkNEjfuecuHXA38541473 = -270479224;    double WdPgDAVGkNEjfuecuHXA80272373 = -440635723;    double WdPgDAVGkNEjfuecuHXA19758306 = -511342631;    double WdPgDAVGkNEjfuecuHXA14024938 = -527122298;    double WdPgDAVGkNEjfuecuHXA16029664 = -229388994;    double WdPgDAVGkNEjfuecuHXA78347447 = -401876235;    double WdPgDAVGkNEjfuecuHXA80535656 = -150614249;    double WdPgDAVGkNEjfuecuHXA56045756 = -916093102;    double WdPgDAVGkNEjfuecuHXA1808681 = -123151441;    double WdPgDAVGkNEjfuecuHXA15069554 = -222566270;    double WdPgDAVGkNEjfuecuHXA63190040 = -89465811;    double WdPgDAVGkNEjfuecuHXA72691489 = -715335357;    double WdPgDAVGkNEjfuecuHXA60292473 = -339416327;    double WdPgDAVGkNEjfuecuHXA95790514 = -322660988;    double WdPgDAVGkNEjfuecuHXA23015958 = -688592245;    double WdPgDAVGkNEjfuecuHXA99847692 = -424045738;    double WdPgDAVGkNEjfuecuHXA74663380 = -612199559;    double WdPgDAVGkNEjfuecuHXA7006632 = -219667156;    double WdPgDAVGkNEjfuecuHXA78621681 = -614632404;    double WdPgDAVGkNEjfuecuHXA83963688 = -694102169;    double WdPgDAVGkNEjfuecuHXA8844357 = -679679130;    double WdPgDAVGkNEjfuecuHXA95672844 = -729706691;    double WdPgDAVGkNEjfuecuHXA17890982 = -808846200;    double WdPgDAVGkNEjfuecuHXA22054745 = -21786405;    double WdPgDAVGkNEjfuecuHXA90765099 = -866740855;    double WdPgDAVGkNEjfuecuHXA81842717 = -148408376;    double WdPgDAVGkNEjfuecuHXA70668281 = -111649337;    double WdPgDAVGkNEjfuecuHXA12322235 = -828103724;    double WdPgDAVGkNEjfuecuHXA47471365 = -639952774;    double WdPgDAVGkNEjfuecuHXA43615749 = -248655058;    double WdPgDAVGkNEjfuecuHXA79350428 = -854122932;    double WdPgDAVGkNEjfuecuHXA71767451 = -90237575;    double WdPgDAVGkNEjfuecuHXA9930027 = -829083;    double WdPgDAVGkNEjfuecuHXA35178203 = -508642724;    double WdPgDAVGkNEjfuecuHXA24865318 = 48998438;    double WdPgDAVGkNEjfuecuHXA86641441 = -394715627;    double WdPgDAVGkNEjfuecuHXA60066917 = -880140811;    double WdPgDAVGkNEjfuecuHXA8582034 = -10330506;    double WdPgDAVGkNEjfuecuHXA12783322 = -155368954;     WdPgDAVGkNEjfuecuHXA89356930 = WdPgDAVGkNEjfuecuHXA92389965;     WdPgDAVGkNEjfuecuHXA92389965 = WdPgDAVGkNEjfuecuHXA14266445;     WdPgDAVGkNEjfuecuHXA14266445 = WdPgDAVGkNEjfuecuHXA21840392;     WdPgDAVGkNEjfuecuHXA21840392 = WdPgDAVGkNEjfuecuHXA12140228;     WdPgDAVGkNEjfuecuHXA12140228 = WdPgDAVGkNEjfuecuHXA53254618;     WdPgDAVGkNEjfuecuHXA53254618 = WdPgDAVGkNEjfuecuHXA71603049;     WdPgDAVGkNEjfuecuHXA71603049 = WdPgDAVGkNEjfuecuHXA94621102;     WdPgDAVGkNEjfuecuHXA94621102 = WdPgDAVGkNEjfuecuHXA89407335;     WdPgDAVGkNEjfuecuHXA89407335 = WdPgDAVGkNEjfuecuHXA6563061;     WdPgDAVGkNEjfuecuHXA6563061 = WdPgDAVGkNEjfuecuHXA34130422;     WdPgDAVGkNEjfuecuHXA34130422 = WdPgDAVGkNEjfuecuHXA60154837;     WdPgDAVGkNEjfuecuHXA60154837 = WdPgDAVGkNEjfuecuHXA95237247;     WdPgDAVGkNEjfuecuHXA95237247 = WdPgDAVGkNEjfuecuHXA13201136;     WdPgDAVGkNEjfuecuHXA13201136 = WdPgDAVGkNEjfuecuHXA40544044;     WdPgDAVGkNEjfuecuHXA40544044 = WdPgDAVGkNEjfuecuHXA72866809;     WdPgDAVGkNEjfuecuHXA72866809 = WdPgDAVGkNEjfuecuHXA23044113;     WdPgDAVGkNEjfuecuHXA23044113 = WdPgDAVGkNEjfuecuHXA38325886;     WdPgDAVGkNEjfuecuHXA38325886 = WdPgDAVGkNEjfuecuHXA31756584;     WdPgDAVGkNEjfuecuHXA31756584 = WdPgDAVGkNEjfuecuHXA59197305;     WdPgDAVGkNEjfuecuHXA59197305 = WdPgDAVGkNEjfuecuHXA89929772;     WdPgDAVGkNEjfuecuHXA89929772 = WdPgDAVGkNEjfuecuHXA79672915;     WdPgDAVGkNEjfuecuHXA79672915 = WdPgDAVGkNEjfuecuHXA97270351;     WdPgDAVGkNEjfuecuHXA97270351 = WdPgDAVGkNEjfuecuHXA35819617;     WdPgDAVGkNEjfuecuHXA35819617 = WdPgDAVGkNEjfuecuHXA33753769;     WdPgDAVGkNEjfuecuHXA33753769 = WdPgDAVGkNEjfuecuHXA56435857;     WdPgDAVGkNEjfuecuHXA56435857 = WdPgDAVGkNEjfuecuHXA16968711;     WdPgDAVGkNEjfuecuHXA16968711 = WdPgDAVGkNEjfuecuHXA33061576;     WdPgDAVGkNEjfuecuHXA33061576 = WdPgDAVGkNEjfuecuHXA14348729;     WdPgDAVGkNEjfuecuHXA14348729 = WdPgDAVGkNEjfuecuHXA69649029;     WdPgDAVGkNEjfuecuHXA69649029 = WdPgDAVGkNEjfuecuHXA92538122;     WdPgDAVGkNEjfuecuHXA92538122 = WdPgDAVGkNEjfuecuHXA18100758;     WdPgDAVGkNEjfuecuHXA18100758 = WdPgDAVGkNEjfuecuHXA81807390;     WdPgDAVGkNEjfuecuHXA81807390 = WdPgDAVGkNEjfuecuHXA14701592;     WdPgDAVGkNEjfuecuHXA14701592 = WdPgDAVGkNEjfuecuHXA57155380;     WdPgDAVGkNEjfuecuHXA57155380 = WdPgDAVGkNEjfuecuHXA38735363;     WdPgDAVGkNEjfuecuHXA38735363 = WdPgDAVGkNEjfuecuHXA57797256;     WdPgDAVGkNEjfuecuHXA57797256 = WdPgDAVGkNEjfuecuHXA59854073;     WdPgDAVGkNEjfuecuHXA59854073 = WdPgDAVGkNEjfuecuHXA65634396;     WdPgDAVGkNEjfuecuHXA65634396 = WdPgDAVGkNEjfuecuHXA71464111;     WdPgDAVGkNEjfuecuHXA71464111 = WdPgDAVGkNEjfuecuHXA63406790;     WdPgDAVGkNEjfuecuHXA63406790 = WdPgDAVGkNEjfuecuHXA66913815;     WdPgDAVGkNEjfuecuHXA66913815 = WdPgDAVGkNEjfuecuHXA79825222;     WdPgDAVGkNEjfuecuHXA79825222 = WdPgDAVGkNEjfuecuHXA22606972;     WdPgDAVGkNEjfuecuHXA22606972 = WdPgDAVGkNEjfuecuHXA28812985;     WdPgDAVGkNEjfuecuHXA28812985 = WdPgDAVGkNEjfuecuHXA55132087;     WdPgDAVGkNEjfuecuHXA55132087 = WdPgDAVGkNEjfuecuHXA72472168;     WdPgDAVGkNEjfuecuHXA72472168 = WdPgDAVGkNEjfuecuHXA8124355;     WdPgDAVGkNEjfuecuHXA8124355 = WdPgDAVGkNEjfuecuHXA37388732;     WdPgDAVGkNEjfuecuHXA37388732 = WdPgDAVGkNEjfuecuHXA96457747;     WdPgDAVGkNEjfuecuHXA96457747 = WdPgDAVGkNEjfuecuHXA47594285;     WdPgDAVGkNEjfuecuHXA47594285 = WdPgDAVGkNEjfuecuHXA1773024;     WdPgDAVGkNEjfuecuHXA1773024 = WdPgDAVGkNEjfuecuHXA36258041;     WdPgDAVGkNEjfuecuHXA36258041 = WdPgDAVGkNEjfuecuHXA11139109;     WdPgDAVGkNEjfuecuHXA11139109 = WdPgDAVGkNEjfuecuHXA2379357;     WdPgDAVGkNEjfuecuHXA2379357 = WdPgDAVGkNEjfuecuHXA9684016;     WdPgDAVGkNEjfuecuHXA9684016 = WdPgDAVGkNEjfuecuHXA95119614;     WdPgDAVGkNEjfuecuHXA95119614 = WdPgDAVGkNEjfuecuHXA78446827;     WdPgDAVGkNEjfuecuHXA78446827 = WdPgDAVGkNEjfuecuHXA88086622;     WdPgDAVGkNEjfuecuHXA88086622 = WdPgDAVGkNEjfuecuHXA55704370;     WdPgDAVGkNEjfuecuHXA55704370 = WdPgDAVGkNEjfuecuHXA36285908;     WdPgDAVGkNEjfuecuHXA36285908 = WdPgDAVGkNEjfuecuHXA38541473;     WdPgDAVGkNEjfuecuHXA38541473 = WdPgDAVGkNEjfuecuHXA80272373;     WdPgDAVGkNEjfuecuHXA80272373 = WdPgDAVGkNEjfuecuHXA19758306;     WdPgDAVGkNEjfuecuHXA19758306 = WdPgDAVGkNEjfuecuHXA14024938;     WdPgDAVGkNEjfuecuHXA14024938 = WdPgDAVGkNEjfuecuHXA16029664;     WdPgDAVGkNEjfuecuHXA16029664 = WdPgDAVGkNEjfuecuHXA78347447;     WdPgDAVGkNEjfuecuHXA78347447 = WdPgDAVGkNEjfuecuHXA80535656;     WdPgDAVGkNEjfuecuHXA80535656 = WdPgDAVGkNEjfuecuHXA56045756;     WdPgDAVGkNEjfuecuHXA56045756 = WdPgDAVGkNEjfuecuHXA1808681;     WdPgDAVGkNEjfuecuHXA1808681 = WdPgDAVGkNEjfuecuHXA15069554;     WdPgDAVGkNEjfuecuHXA15069554 = WdPgDAVGkNEjfuecuHXA63190040;     WdPgDAVGkNEjfuecuHXA63190040 = WdPgDAVGkNEjfuecuHXA72691489;     WdPgDAVGkNEjfuecuHXA72691489 = WdPgDAVGkNEjfuecuHXA60292473;     WdPgDAVGkNEjfuecuHXA60292473 = WdPgDAVGkNEjfuecuHXA95790514;     WdPgDAVGkNEjfuecuHXA95790514 = WdPgDAVGkNEjfuecuHXA23015958;     WdPgDAVGkNEjfuecuHXA23015958 = WdPgDAVGkNEjfuecuHXA99847692;     WdPgDAVGkNEjfuecuHXA99847692 = WdPgDAVGkNEjfuecuHXA74663380;     WdPgDAVGkNEjfuecuHXA74663380 = WdPgDAVGkNEjfuecuHXA7006632;     WdPgDAVGkNEjfuecuHXA7006632 = WdPgDAVGkNEjfuecuHXA78621681;     WdPgDAVGkNEjfuecuHXA78621681 = WdPgDAVGkNEjfuecuHXA83963688;     WdPgDAVGkNEjfuecuHXA83963688 = WdPgDAVGkNEjfuecuHXA8844357;     WdPgDAVGkNEjfuecuHXA8844357 = WdPgDAVGkNEjfuecuHXA95672844;     WdPgDAVGkNEjfuecuHXA95672844 = WdPgDAVGkNEjfuecuHXA17890982;     WdPgDAVGkNEjfuecuHXA17890982 = WdPgDAVGkNEjfuecuHXA22054745;     WdPgDAVGkNEjfuecuHXA22054745 = WdPgDAVGkNEjfuecuHXA90765099;     WdPgDAVGkNEjfuecuHXA90765099 = WdPgDAVGkNEjfuecuHXA81842717;     WdPgDAVGkNEjfuecuHXA81842717 = WdPgDAVGkNEjfuecuHXA70668281;     WdPgDAVGkNEjfuecuHXA70668281 = WdPgDAVGkNEjfuecuHXA12322235;     WdPgDAVGkNEjfuecuHXA12322235 = WdPgDAVGkNEjfuecuHXA47471365;     WdPgDAVGkNEjfuecuHXA47471365 = WdPgDAVGkNEjfuecuHXA43615749;     WdPgDAVGkNEjfuecuHXA43615749 = WdPgDAVGkNEjfuecuHXA79350428;     WdPgDAVGkNEjfuecuHXA79350428 = WdPgDAVGkNEjfuecuHXA71767451;     WdPgDAVGkNEjfuecuHXA71767451 = WdPgDAVGkNEjfuecuHXA9930027;     WdPgDAVGkNEjfuecuHXA9930027 = WdPgDAVGkNEjfuecuHXA35178203;     WdPgDAVGkNEjfuecuHXA35178203 = WdPgDAVGkNEjfuecuHXA24865318;     WdPgDAVGkNEjfuecuHXA24865318 = WdPgDAVGkNEjfuecuHXA86641441;     WdPgDAVGkNEjfuecuHXA86641441 = WdPgDAVGkNEjfuecuHXA60066917;     WdPgDAVGkNEjfuecuHXA60066917 = WdPgDAVGkNEjfuecuHXA8582034;     WdPgDAVGkNEjfuecuHXA8582034 = WdPgDAVGkNEjfuecuHXA12783322;     WdPgDAVGkNEjfuecuHXA12783322 = WdPgDAVGkNEjfuecuHXA89356930;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NTLJQOeyEIWpSvnmHbyT19903386() {     double RRHpGCAclwExmLmSquUM14942208 = -680402055;    double RRHpGCAclwExmLmSquUM88787109 = -244176143;    double RRHpGCAclwExmLmSquUM47145620 = -229596958;    double RRHpGCAclwExmLmSquUM67562321 = -615942335;    double RRHpGCAclwExmLmSquUM81482232 = -151652157;    double RRHpGCAclwExmLmSquUM70263895 = -226534307;    double RRHpGCAclwExmLmSquUM76923893 = -162374333;    double RRHpGCAclwExmLmSquUM9631227 = -545060636;    double RRHpGCAclwExmLmSquUM14736628 = -227389509;    double RRHpGCAclwExmLmSquUM25721443 = -461564137;    double RRHpGCAclwExmLmSquUM48463976 = -102228341;    double RRHpGCAclwExmLmSquUM95472492 = -118690333;    double RRHpGCAclwExmLmSquUM71818026 = 21593794;    double RRHpGCAclwExmLmSquUM4760853 = -870409944;    double RRHpGCAclwExmLmSquUM96812931 = -99398458;    double RRHpGCAclwExmLmSquUM80587852 = -34334408;    double RRHpGCAclwExmLmSquUM22842230 = -216764877;    double RRHpGCAclwExmLmSquUM59441791 = -830082049;    double RRHpGCAclwExmLmSquUM93221305 = -708234690;    double RRHpGCAclwExmLmSquUM20358772 = -718581149;    double RRHpGCAclwExmLmSquUM90939038 = 14356625;    double RRHpGCAclwExmLmSquUM81176021 = -235262109;    double RRHpGCAclwExmLmSquUM1034150 = -293744468;    double RRHpGCAclwExmLmSquUM37533659 = -764530672;    double RRHpGCAclwExmLmSquUM88656908 = 72626114;    double RRHpGCAclwExmLmSquUM85924994 = -171540163;    double RRHpGCAclwExmLmSquUM99356619 = -660164656;    double RRHpGCAclwExmLmSquUM96960376 = -539286135;    double RRHpGCAclwExmLmSquUM22225941 = -960036885;    double RRHpGCAclwExmLmSquUM22971521 = -128732395;    double RRHpGCAclwExmLmSquUM14584757 = -378527347;    double RRHpGCAclwExmLmSquUM55290691 = -836207295;    double RRHpGCAclwExmLmSquUM2994442 = -720926329;    double RRHpGCAclwExmLmSquUM68288663 = -19688564;    double RRHpGCAclwExmLmSquUM58560368 = -581228460;    double RRHpGCAclwExmLmSquUM43527213 = -505295777;    double RRHpGCAclwExmLmSquUM7442642 = -856088606;    double RRHpGCAclwExmLmSquUM35426760 = -187694098;    double RRHpGCAclwExmLmSquUM6557976 = 43192346;    double RRHpGCAclwExmLmSquUM80247785 = 44185175;    double RRHpGCAclwExmLmSquUM10558072 = -896540975;    double RRHpGCAclwExmLmSquUM10923405 = -344389854;    double RRHpGCAclwExmLmSquUM29025442 = -420089823;    double RRHpGCAclwExmLmSquUM93400250 = -5868403;    double RRHpGCAclwExmLmSquUM20793862 = -525035654;    double RRHpGCAclwExmLmSquUM21532410 = -167792878;    double RRHpGCAclwExmLmSquUM72240623 = -945456313;    double RRHpGCAclwExmLmSquUM16237126 = -507855239;    double RRHpGCAclwExmLmSquUM88200051 = -679593439;    double RRHpGCAclwExmLmSquUM50267742 = -762919988;    double RRHpGCAclwExmLmSquUM57730532 = -346695031;    double RRHpGCAclwExmLmSquUM29031603 = -911218640;    double RRHpGCAclwExmLmSquUM54331644 = -881901634;    double RRHpGCAclwExmLmSquUM80038017 = -376519637;    double RRHpGCAclwExmLmSquUM28964348 = -610530287;    double RRHpGCAclwExmLmSquUM33766186 = -345139947;    double RRHpGCAclwExmLmSquUM87752959 = -950431675;    double RRHpGCAclwExmLmSquUM9611961 = -465066287;    double RRHpGCAclwExmLmSquUM78905413 = -588568449;    double RRHpGCAclwExmLmSquUM95557238 = -980111995;    double RRHpGCAclwExmLmSquUM70907275 = -566369651;    double RRHpGCAclwExmLmSquUM79963517 = -623088199;    double RRHpGCAclwExmLmSquUM87405285 = -585023752;    double RRHpGCAclwExmLmSquUM91765107 = 1342886;    double RRHpGCAclwExmLmSquUM11136686 = 16963209;    double RRHpGCAclwExmLmSquUM93173285 = -266021047;    double RRHpGCAclwExmLmSquUM92478051 = -397764005;    double RRHpGCAclwExmLmSquUM3529363 = -958717642;    double RRHpGCAclwExmLmSquUM46200485 = -189181484;    double RRHpGCAclwExmLmSquUM53285718 = -594102682;    double RRHpGCAclwExmLmSquUM73145211 = -178245802;    double RRHpGCAclwExmLmSquUM87415469 = 70929220;    double RRHpGCAclwExmLmSquUM52883815 = -773274396;    double RRHpGCAclwExmLmSquUM12973520 = -652419865;    double RRHpGCAclwExmLmSquUM9800700 = -822040174;    double RRHpGCAclwExmLmSquUM80015633 = -641253522;    double RRHpGCAclwExmLmSquUM52150580 = -815172286;    double RRHpGCAclwExmLmSquUM7633899 = -187876066;    double RRHpGCAclwExmLmSquUM16739798 = -139495018;    double RRHpGCAclwExmLmSquUM67124499 = -759581009;    double RRHpGCAclwExmLmSquUM13684371 = -226083850;    double RRHpGCAclwExmLmSquUM83119494 = -52309418;    double RRHpGCAclwExmLmSquUM8760325 = -859692697;    double RRHpGCAclwExmLmSquUM71958199 = -97116898;    double RRHpGCAclwExmLmSquUM65240988 = -782037365;    double RRHpGCAclwExmLmSquUM85553153 = -467308707;    double RRHpGCAclwExmLmSquUM959047 = -954305661;    double RRHpGCAclwExmLmSquUM22956425 = -244406693;    double RRHpGCAclwExmLmSquUM39324315 = -409158278;    double RRHpGCAclwExmLmSquUM24794183 = -136088514;    double RRHpGCAclwExmLmSquUM55774254 = -554864102;    double RRHpGCAclwExmLmSquUM97830680 = -291022320;    double RRHpGCAclwExmLmSquUM56521347 = -599125649;    double RRHpGCAclwExmLmSquUM11000737 = 23304341;    double RRHpGCAclwExmLmSquUM9340510 = -389445174;    double RRHpGCAclwExmLmSquUM30594554 = -173452777;    double RRHpGCAclwExmLmSquUM23518119 = -759366102;    double RRHpGCAclwExmLmSquUM37260334 = -321432710;    double RRHpGCAclwExmLmSquUM82263565 = 77168388;    double RRHpGCAclwExmLmSquUM27620576 = -680402055;     RRHpGCAclwExmLmSquUM14942208 = RRHpGCAclwExmLmSquUM88787109;     RRHpGCAclwExmLmSquUM88787109 = RRHpGCAclwExmLmSquUM47145620;     RRHpGCAclwExmLmSquUM47145620 = RRHpGCAclwExmLmSquUM67562321;     RRHpGCAclwExmLmSquUM67562321 = RRHpGCAclwExmLmSquUM81482232;     RRHpGCAclwExmLmSquUM81482232 = RRHpGCAclwExmLmSquUM70263895;     RRHpGCAclwExmLmSquUM70263895 = RRHpGCAclwExmLmSquUM76923893;     RRHpGCAclwExmLmSquUM76923893 = RRHpGCAclwExmLmSquUM9631227;     RRHpGCAclwExmLmSquUM9631227 = RRHpGCAclwExmLmSquUM14736628;     RRHpGCAclwExmLmSquUM14736628 = RRHpGCAclwExmLmSquUM25721443;     RRHpGCAclwExmLmSquUM25721443 = RRHpGCAclwExmLmSquUM48463976;     RRHpGCAclwExmLmSquUM48463976 = RRHpGCAclwExmLmSquUM95472492;     RRHpGCAclwExmLmSquUM95472492 = RRHpGCAclwExmLmSquUM71818026;     RRHpGCAclwExmLmSquUM71818026 = RRHpGCAclwExmLmSquUM4760853;     RRHpGCAclwExmLmSquUM4760853 = RRHpGCAclwExmLmSquUM96812931;     RRHpGCAclwExmLmSquUM96812931 = RRHpGCAclwExmLmSquUM80587852;     RRHpGCAclwExmLmSquUM80587852 = RRHpGCAclwExmLmSquUM22842230;     RRHpGCAclwExmLmSquUM22842230 = RRHpGCAclwExmLmSquUM59441791;     RRHpGCAclwExmLmSquUM59441791 = RRHpGCAclwExmLmSquUM93221305;     RRHpGCAclwExmLmSquUM93221305 = RRHpGCAclwExmLmSquUM20358772;     RRHpGCAclwExmLmSquUM20358772 = RRHpGCAclwExmLmSquUM90939038;     RRHpGCAclwExmLmSquUM90939038 = RRHpGCAclwExmLmSquUM81176021;     RRHpGCAclwExmLmSquUM81176021 = RRHpGCAclwExmLmSquUM1034150;     RRHpGCAclwExmLmSquUM1034150 = RRHpGCAclwExmLmSquUM37533659;     RRHpGCAclwExmLmSquUM37533659 = RRHpGCAclwExmLmSquUM88656908;     RRHpGCAclwExmLmSquUM88656908 = RRHpGCAclwExmLmSquUM85924994;     RRHpGCAclwExmLmSquUM85924994 = RRHpGCAclwExmLmSquUM99356619;     RRHpGCAclwExmLmSquUM99356619 = RRHpGCAclwExmLmSquUM96960376;     RRHpGCAclwExmLmSquUM96960376 = RRHpGCAclwExmLmSquUM22225941;     RRHpGCAclwExmLmSquUM22225941 = RRHpGCAclwExmLmSquUM22971521;     RRHpGCAclwExmLmSquUM22971521 = RRHpGCAclwExmLmSquUM14584757;     RRHpGCAclwExmLmSquUM14584757 = RRHpGCAclwExmLmSquUM55290691;     RRHpGCAclwExmLmSquUM55290691 = RRHpGCAclwExmLmSquUM2994442;     RRHpGCAclwExmLmSquUM2994442 = RRHpGCAclwExmLmSquUM68288663;     RRHpGCAclwExmLmSquUM68288663 = RRHpGCAclwExmLmSquUM58560368;     RRHpGCAclwExmLmSquUM58560368 = RRHpGCAclwExmLmSquUM43527213;     RRHpGCAclwExmLmSquUM43527213 = RRHpGCAclwExmLmSquUM7442642;     RRHpGCAclwExmLmSquUM7442642 = RRHpGCAclwExmLmSquUM35426760;     RRHpGCAclwExmLmSquUM35426760 = RRHpGCAclwExmLmSquUM6557976;     RRHpGCAclwExmLmSquUM6557976 = RRHpGCAclwExmLmSquUM80247785;     RRHpGCAclwExmLmSquUM80247785 = RRHpGCAclwExmLmSquUM10558072;     RRHpGCAclwExmLmSquUM10558072 = RRHpGCAclwExmLmSquUM10923405;     RRHpGCAclwExmLmSquUM10923405 = RRHpGCAclwExmLmSquUM29025442;     RRHpGCAclwExmLmSquUM29025442 = RRHpGCAclwExmLmSquUM93400250;     RRHpGCAclwExmLmSquUM93400250 = RRHpGCAclwExmLmSquUM20793862;     RRHpGCAclwExmLmSquUM20793862 = RRHpGCAclwExmLmSquUM21532410;     RRHpGCAclwExmLmSquUM21532410 = RRHpGCAclwExmLmSquUM72240623;     RRHpGCAclwExmLmSquUM72240623 = RRHpGCAclwExmLmSquUM16237126;     RRHpGCAclwExmLmSquUM16237126 = RRHpGCAclwExmLmSquUM88200051;     RRHpGCAclwExmLmSquUM88200051 = RRHpGCAclwExmLmSquUM50267742;     RRHpGCAclwExmLmSquUM50267742 = RRHpGCAclwExmLmSquUM57730532;     RRHpGCAclwExmLmSquUM57730532 = RRHpGCAclwExmLmSquUM29031603;     RRHpGCAclwExmLmSquUM29031603 = RRHpGCAclwExmLmSquUM54331644;     RRHpGCAclwExmLmSquUM54331644 = RRHpGCAclwExmLmSquUM80038017;     RRHpGCAclwExmLmSquUM80038017 = RRHpGCAclwExmLmSquUM28964348;     RRHpGCAclwExmLmSquUM28964348 = RRHpGCAclwExmLmSquUM33766186;     RRHpGCAclwExmLmSquUM33766186 = RRHpGCAclwExmLmSquUM87752959;     RRHpGCAclwExmLmSquUM87752959 = RRHpGCAclwExmLmSquUM9611961;     RRHpGCAclwExmLmSquUM9611961 = RRHpGCAclwExmLmSquUM78905413;     RRHpGCAclwExmLmSquUM78905413 = RRHpGCAclwExmLmSquUM95557238;     RRHpGCAclwExmLmSquUM95557238 = RRHpGCAclwExmLmSquUM70907275;     RRHpGCAclwExmLmSquUM70907275 = RRHpGCAclwExmLmSquUM79963517;     RRHpGCAclwExmLmSquUM79963517 = RRHpGCAclwExmLmSquUM87405285;     RRHpGCAclwExmLmSquUM87405285 = RRHpGCAclwExmLmSquUM91765107;     RRHpGCAclwExmLmSquUM91765107 = RRHpGCAclwExmLmSquUM11136686;     RRHpGCAclwExmLmSquUM11136686 = RRHpGCAclwExmLmSquUM93173285;     RRHpGCAclwExmLmSquUM93173285 = RRHpGCAclwExmLmSquUM92478051;     RRHpGCAclwExmLmSquUM92478051 = RRHpGCAclwExmLmSquUM3529363;     RRHpGCAclwExmLmSquUM3529363 = RRHpGCAclwExmLmSquUM46200485;     RRHpGCAclwExmLmSquUM46200485 = RRHpGCAclwExmLmSquUM53285718;     RRHpGCAclwExmLmSquUM53285718 = RRHpGCAclwExmLmSquUM73145211;     RRHpGCAclwExmLmSquUM73145211 = RRHpGCAclwExmLmSquUM87415469;     RRHpGCAclwExmLmSquUM87415469 = RRHpGCAclwExmLmSquUM52883815;     RRHpGCAclwExmLmSquUM52883815 = RRHpGCAclwExmLmSquUM12973520;     RRHpGCAclwExmLmSquUM12973520 = RRHpGCAclwExmLmSquUM9800700;     RRHpGCAclwExmLmSquUM9800700 = RRHpGCAclwExmLmSquUM80015633;     RRHpGCAclwExmLmSquUM80015633 = RRHpGCAclwExmLmSquUM52150580;     RRHpGCAclwExmLmSquUM52150580 = RRHpGCAclwExmLmSquUM7633899;     RRHpGCAclwExmLmSquUM7633899 = RRHpGCAclwExmLmSquUM16739798;     RRHpGCAclwExmLmSquUM16739798 = RRHpGCAclwExmLmSquUM67124499;     RRHpGCAclwExmLmSquUM67124499 = RRHpGCAclwExmLmSquUM13684371;     RRHpGCAclwExmLmSquUM13684371 = RRHpGCAclwExmLmSquUM83119494;     RRHpGCAclwExmLmSquUM83119494 = RRHpGCAclwExmLmSquUM8760325;     RRHpGCAclwExmLmSquUM8760325 = RRHpGCAclwExmLmSquUM71958199;     RRHpGCAclwExmLmSquUM71958199 = RRHpGCAclwExmLmSquUM65240988;     RRHpGCAclwExmLmSquUM65240988 = RRHpGCAclwExmLmSquUM85553153;     RRHpGCAclwExmLmSquUM85553153 = RRHpGCAclwExmLmSquUM959047;     RRHpGCAclwExmLmSquUM959047 = RRHpGCAclwExmLmSquUM22956425;     RRHpGCAclwExmLmSquUM22956425 = RRHpGCAclwExmLmSquUM39324315;     RRHpGCAclwExmLmSquUM39324315 = RRHpGCAclwExmLmSquUM24794183;     RRHpGCAclwExmLmSquUM24794183 = RRHpGCAclwExmLmSquUM55774254;     RRHpGCAclwExmLmSquUM55774254 = RRHpGCAclwExmLmSquUM97830680;     RRHpGCAclwExmLmSquUM97830680 = RRHpGCAclwExmLmSquUM56521347;     RRHpGCAclwExmLmSquUM56521347 = RRHpGCAclwExmLmSquUM11000737;     RRHpGCAclwExmLmSquUM11000737 = RRHpGCAclwExmLmSquUM9340510;     RRHpGCAclwExmLmSquUM9340510 = RRHpGCAclwExmLmSquUM30594554;     RRHpGCAclwExmLmSquUM30594554 = RRHpGCAclwExmLmSquUM23518119;     RRHpGCAclwExmLmSquUM23518119 = RRHpGCAclwExmLmSquUM37260334;     RRHpGCAclwExmLmSquUM37260334 = RRHpGCAclwExmLmSquUM82263565;     RRHpGCAclwExmLmSquUM82263565 = RRHpGCAclwExmLmSquUM27620576;     RRHpGCAclwExmLmSquUM27620576 = RRHpGCAclwExmLmSquUM14942208;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lDkXFfcIjyCxchkQgelT47497643() {     double TCIUIkjdZnLxmwpNnlPc40527484 = -105435155;    double TCIUIkjdZnLxmwpNnlPc85184252 = -499653919;    double TCIUIkjdZnLxmwpNnlPc80024795 = -32058494;    double TCIUIkjdZnLxmwpNnlPc13284251 = -623265229;    double TCIUIkjdZnLxmwpNnlPc50824237 = -198574444;    double TCIUIkjdZnLxmwpNnlPc87273171 = -381515310;    double TCIUIkjdZnLxmwpNnlPc82244738 = -744645540;    double TCIUIkjdZnLxmwpNnlPc24641351 = -925742515;    double TCIUIkjdZnLxmwpNnlPc40065920 = -723613553;    double TCIUIkjdZnLxmwpNnlPc44879825 = -796125660;    double TCIUIkjdZnLxmwpNnlPc62797529 = -412656228;    double TCIUIkjdZnLxmwpNnlPc30790149 = -562399240;    double TCIUIkjdZnLxmwpNnlPc48398804 = -842654571;    double TCIUIkjdZnLxmwpNnlPc96320569 = -844934237;    double TCIUIkjdZnLxmwpNnlPc53081820 = 12055584;    double TCIUIkjdZnLxmwpNnlPc88308894 = -470525885;    double TCIUIkjdZnLxmwpNnlPc22640346 = 94915329;    double TCIUIkjdZnLxmwpNnlPc80557695 = -246359410;    double TCIUIkjdZnLxmwpNnlPc54686026 = -391208547;    double TCIUIkjdZnLxmwpNnlPc81520238 = -693020525;    double TCIUIkjdZnLxmwpNnlPc91948303 = -447343157;    double TCIUIkjdZnLxmwpNnlPc82679128 = -654995038;    double TCIUIkjdZnLxmwpNnlPc4797948 = -337836573;    double TCIUIkjdZnLxmwpNnlPc39247702 = -623379651;    double TCIUIkjdZnLxmwpNnlPc43560048 = -594870029;    double TCIUIkjdZnLxmwpNnlPc15414131 = -835990704;    double TCIUIkjdZnLxmwpNnlPc81744528 = -225977791;    double TCIUIkjdZnLxmwpNnlPc60859176 = -768948368;    double TCIUIkjdZnLxmwpNnlPc30103154 = -96330735;    double TCIUIkjdZnLxmwpNnlPc76294011 = -37641957;    double TCIUIkjdZnLxmwpNnlPc36631392 = -157174377;    double TCIUIkjdZnLxmwpNnlPc92480623 = -10003129;    double TCIUIkjdZnLxmwpNnlPc24181494 = -68747466;    double TCIUIkjdZnLxmwpNnlPc21875734 = -75833536;    double TCIUIkjdZnLxmwpNnlPc59965356 = -182664372;    double TCIUIkjdZnLxmwpNnlPc48319064 = 77109507;    double TCIUIkjdZnLxmwpNnlPc57088026 = -236600551;    double TCIUIkjdZnLxmwpNnlPc10999447 = -36408923;    double TCIUIkjdZnLxmwpNnlPc47481554 = -415145976;    double TCIUIkjdZnLxmwpNnlPc89031458 = -425785144;    double TCIUIkjdZnLxmwpNnlPc57709352 = -371601165;    double TCIUIkjdZnLxmwpNnlPc54932995 = -853428358;    double TCIUIkjdZnLxmwpNnlPc78225661 = -448696205;    double TCIUIkjdZnLxmwpNnlPc64193530 = -474284002;    double TCIUIkjdZnLxmwpNnlPc12774738 = -464056771;    double TCIUIkjdZnLxmwpNnlPc87932731 = -690340416;    double TCIUIkjdZnLxmwpNnlPc72009077 = -977925173;    double TCIUIkjdZnLxmwpNnlPc24349896 = -701038085;    double TCIUIkjdZnLxmwpNnlPc39011372 = -779269665;    double TCIUIkjdZnLxmwpNnlPc4077738 = -610943141;    double TCIUIkjdZnLxmwpNnlPc67866780 = -595353633;    double TCIUIkjdZnLxmwpNnlPc56290183 = 10702180;    double TCIUIkjdZnLxmwpNnlPc72405248 = -349800183;    double TCIUIkjdZnLxmwpNnlPc48936926 = -691583418;    double TCIUIkjdZnLxmwpNnlPc55549339 = 14379296;    double TCIUIkjdZnLxmwpNnlPc57848356 = -450440118;    double TCIUIkjdZnLxmwpNnlPc80386305 = -61817347;    double TCIUIkjdZnLxmwpNnlPc40777094 = -408678843;    double TCIUIkjdZnLxmwpNnlPc69724203 = 71604800;    double TCIUIkjdZnLxmwpNnlPc35410107 = -362583741;    double TCIUIkjdZnLxmwpNnlPc5528643 = -55537520;    double TCIUIkjdZnLxmwpNnlPc21385562 = -975697173;    double TCIUIkjdZnLxmwpNnlPc94538197 = -729411781;    double TCIUIkjdZnLxmwpNnlPc63771909 = -585971597;    double TCIUIkjdZnLxmwpNnlPc8248434 = -538951284;    double TCIUIkjdZnLxmwpNnlPc70316906 = -302653100;    double TCIUIkjdZnLxmwpNnlPc6608655 = -393651774;    double TCIUIkjdZnLxmwpNnlPc26523070 = -666821036;    double TCIUIkjdZnLxmwpNnlPc36355214 = -562269866;    double TCIUIkjdZnLxmwpNnlPc4762757 = 34946076;    double TCIUIkjdZnLxmwpNnlPc31220869 = -133925335;    double TCIUIkjdZnLxmwpNnlPc11640899 = -868675749;    double TCIUIkjdZnLxmwpNnlPc33076141 = -831213435;    double TCIUIkjdZnLxmwpNnlPc65654567 = -965423404;    double TCIUIkjdZnLxmwpNnlPc23810886 = -221419360;    double TCIUIkjdZnLxmwpNnlPc37015309 = -593914799;    double TCIUIkjdZnLxmwpNnlPc4453468 = -106298834;    double TCIUIkjdZnLxmwpNnlPc40604418 = -863552572;    double TCIUIkjdZnLxmwpNnlPc26472964 = -59322881;    double TCIUIkjdZnLxmwpNnlPc55627316 = -904529613;    double TCIUIkjdZnLxmwpNnlPc43405054 = -858065531;    double TCIUIkjdZnLxmwpNnlPc57394632 = -524939706;    double TCIUIkjdZnLxmwpNnlPc21847805 = -989678703;    double TCIUIkjdZnLxmwpNnlPc26025417 = -485387595;    double TCIUIkjdZnLxmwpNnlPc8427232 = -442288324;    double TCIUIkjdZnLxmwpNnlPc80341208 = -67876558;    double TCIUIkjdZnLxmwpNnlPc20075376 = -660202946;    double TCIUIkjdZnLxmwpNnlPc75244567 = -377164048;    double TCIUIkjdZnLxmwpNnlPc66326394 = 9787168;    double TCIUIkjdZnLxmwpNnlPc2117001 = -732224255;    double TCIUIkjdZnLxmwpNnlPc67932758 = -861073146;    double TCIUIkjdZnLxmwpNnlPc16310933 = -827921708;    double TCIUIkjdZnLxmwpNnlPc41275244 = -8013723;    double TCIUIkjdZnLxmwpNnlPc12071448 = 47437764;    double TCIUIkjdZnLxmwpNnlPc83502816 = -270247625;    double TCIUIkjdZnLxmwpNnlPc36323791 = -395903993;    double TCIUIkjdZnLxmwpNnlPc60394797 = -24016578;    double TCIUIkjdZnLxmwpNnlPc14453752 = -862724608;    double TCIUIkjdZnLxmwpNnlPc55945096 = -935332719;    double TCIUIkjdZnLxmwpNnlPc42457831 = -105435155;     TCIUIkjdZnLxmwpNnlPc40527484 = TCIUIkjdZnLxmwpNnlPc85184252;     TCIUIkjdZnLxmwpNnlPc85184252 = TCIUIkjdZnLxmwpNnlPc80024795;     TCIUIkjdZnLxmwpNnlPc80024795 = TCIUIkjdZnLxmwpNnlPc13284251;     TCIUIkjdZnLxmwpNnlPc13284251 = TCIUIkjdZnLxmwpNnlPc50824237;     TCIUIkjdZnLxmwpNnlPc50824237 = TCIUIkjdZnLxmwpNnlPc87273171;     TCIUIkjdZnLxmwpNnlPc87273171 = TCIUIkjdZnLxmwpNnlPc82244738;     TCIUIkjdZnLxmwpNnlPc82244738 = TCIUIkjdZnLxmwpNnlPc24641351;     TCIUIkjdZnLxmwpNnlPc24641351 = TCIUIkjdZnLxmwpNnlPc40065920;     TCIUIkjdZnLxmwpNnlPc40065920 = TCIUIkjdZnLxmwpNnlPc44879825;     TCIUIkjdZnLxmwpNnlPc44879825 = TCIUIkjdZnLxmwpNnlPc62797529;     TCIUIkjdZnLxmwpNnlPc62797529 = TCIUIkjdZnLxmwpNnlPc30790149;     TCIUIkjdZnLxmwpNnlPc30790149 = TCIUIkjdZnLxmwpNnlPc48398804;     TCIUIkjdZnLxmwpNnlPc48398804 = TCIUIkjdZnLxmwpNnlPc96320569;     TCIUIkjdZnLxmwpNnlPc96320569 = TCIUIkjdZnLxmwpNnlPc53081820;     TCIUIkjdZnLxmwpNnlPc53081820 = TCIUIkjdZnLxmwpNnlPc88308894;     TCIUIkjdZnLxmwpNnlPc88308894 = TCIUIkjdZnLxmwpNnlPc22640346;     TCIUIkjdZnLxmwpNnlPc22640346 = TCIUIkjdZnLxmwpNnlPc80557695;     TCIUIkjdZnLxmwpNnlPc80557695 = TCIUIkjdZnLxmwpNnlPc54686026;     TCIUIkjdZnLxmwpNnlPc54686026 = TCIUIkjdZnLxmwpNnlPc81520238;     TCIUIkjdZnLxmwpNnlPc81520238 = TCIUIkjdZnLxmwpNnlPc91948303;     TCIUIkjdZnLxmwpNnlPc91948303 = TCIUIkjdZnLxmwpNnlPc82679128;     TCIUIkjdZnLxmwpNnlPc82679128 = TCIUIkjdZnLxmwpNnlPc4797948;     TCIUIkjdZnLxmwpNnlPc4797948 = TCIUIkjdZnLxmwpNnlPc39247702;     TCIUIkjdZnLxmwpNnlPc39247702 = TCIUIkjdZnLxmwpNnlPc43560048;     TCIUIkjdZnLxmwpNnlPc43560048 = TCIUIkjdZnLxmwpNnlPc15414131;     TCIUIkjdZnLxmwpNnlPc15414131 = TCIUIkjdZnLxmwpNnlPc81744528;     TCIUIkjdZnLxmwpNnlPc81744528 = TCIUIkjdZnLxmwpNnlPc60859176;     TCIUIkjdZnLxmwpNnlPc60859176 = TCIUIkjdZnLxmwpNnlPc30103154;     TCIUIkjdZnLxmwpNnlPc30103154 = TCIUIkjdZnLxmwpNnlPc76294011;     TCIUIkjdZnLxmwpNnlPc76294011 = TCIUIkjdZnLxmwpNnlPc36631392;     TCIUIkjdZnLxmwpNnlPc36631392 = TCIUIkjdZnLxmwpNnlPc92480623;     TCIUIkjdZnLxmwpNnlPc92480623 = TCIUIkjdZnLxmwpNnlPc24181494;     TCIUIkjdZnLxmwpNnlPc24181494 = TCIUIkjdZnLxmwpNnlPc21875734;     TCIUIkjdZnLxmwpNnlPc21875734 = TCIUIkjdZnLxmwpNnlPc59965356;     TCIUIkjdZnLxmwpNnlPc59965356 = TCIUIkjdZnLxmwpNnlPc48319064;     TCIUIkjdZnLxmwpNnlPc48319064 = TCIUIkjdZnLxmwpNnlPc57088026;     TCIUIkjdZnLxmwpNnlPc57088026 = TCIUIkjdZnLxmwpNnlPc10999447;     TCIUIkjdZnLxmwpNnlPc10999447 = TCIUIkjdZnLxmwpNnlPc47481554;     TCIUIkjdZnLxmwpNnlPc47481554 = TCIUIkjdZnLxmwpNnlPc89031458;     TCIUIkjdZnLxmwpNnlPc89031458 = TCIUIkjdZnLxmwpNnlPc57709352;     TCIUIkjdZnLxmwpNnlPc57709352 = TCIUIkjdZnLxmwpNnlPc54932995;     TCIUIkjdZnLxmwpNnlPc54932995 = TCIUIkjdZnLxmwpNnlPc78225661;     TCIUIkjdZnLxmwpNnlPc78225661 = TCIUIkjdZnLxmwpNnlPc64193530;     TCIUIkjdZnLxmwpNnlPc64193530 = TCIUIkjdZnLxmwpNnlPc12774738;     TCIUIkjdZnLxmwpNnlPc12774738 = TCIUIkjdZnLxmwpNnlPc87932731;     TCIUIkjdZnLxmwpNnlPc87932731 = TCIUIkjdZnLxmwpNnlPc72009077;     TCIUIkjdZnLxmwpNnlPc72009077 = TCIUIkjdZnLxmwpNnlPc24349896;     TCIUIkjdZnLxmwpNnlPc24349896 = TCIUIkjdZnLxmwpNnlPc39011372;     TCIUIkjdZnLxmwpNnlPc39011372 = TCIUIkjdZnLxmwpNnlPc4077738;     TCIUIkjdZnLxmwpNnlPc4077738 = TCIUIkjdZnLxmwpNnlPc67866780;     TCIUIkjdZnLxmwpNnlPc67866780 = TCIUIkjdZnLxmwpNnlPc56290183;     TCIUIkjdZnLxmwpNnlPc56290183 = TCIUIkjdZnLxmwpNnlPc72405248;     TCIUIkjdZnLxmwpNnlPc72405248 = TCIUIkjdZnLxmwpNnlPc48936926;     TCIUIkjdZnLxmwpNnlPc48936926 = TCIUIkjdZnLxmwpNnlPc55549339;     TCIUIkjdZnLxmwpNnlPc55549339 = TCIUIkjdZnLxmwpNnlPc57848356;     TCIUIkjdZnLxmwpNnlPc57848356 = TCIUIkjdZnLxmwpNnlPc80386305;     TCIUIkjdZnLxmwpNnlPc80386305 = TCIUIkjdZnLxmwpNnlPc40777094;     TCIUIkjdZnLxmwpNnlPc40777094 = TCIUIkjdZnLxmwpNnlPc69724203;     TCIUIkjdZnLxmwpNnlPc69724203 = TCIUIkjdZnLxmwpNnlPc35410107;     TCIUIkjdZnLxmwpNnlPc35410107 = TCIUIkjdZnLxmwpNnlPc5528643;     TCIUIkjdZnLxmwpNnlPc5528643 = TCIUIkjdZnLxmwpNnlPc21385562;     TCIUIkjdZnLxmwpNnlPc21385562 = TCIUIkjdZnLxmwpNnlPc94538197;     TCIUIkjdZnLxmwpNnlPc94538197 = TCIUIkjdZnLxmwpNnlPc63771909;     TCIUIkjdZnLxmwpNnlPc63771909 = TCIUIkjdZnLxmwpNnlPc8248434;     TCIUIkjdZnLxmwpNnlPc8248434 = TCIUIkjdZnLxmwpNnlPc70316906;     TCIUIkjdZnLxmwpNnlPc70316906 = TCIUIkjdZnLxmwpNnlPc6608655;     TCIUIkjdZnLxmwpNnlPc6608655 = TCIUIkjdZnLxmwpNnlPc26523070;     TCIUIkjdZnLxmwpNnlPc26523070 = TCIUIkjdZnLxmwpNnlPc36355214;     TCIUIkjdZnLxmwpNnlPc36355214 = TCIUIkjdZnLxmwpNnlPc4762757;     TCIUIkjdZnLxmwpNnlPc4762757 = TCIUIkjdZnLxmwpNnlPc31220869;     TCIUIkjdZnLxmwpNnlPc31220869 = TCIUIkjdZnLxmwpNnlPc11640899;     TCIUIkjdZnLxmwpNnlPc11640899 = TCIUIkjdZnLxmwpNnlPc33076141;     TCIUIkjdZnLxmwpNnlPc33076141 = TCIUIkjdZnLxmwpNnlPc65654567;     TCIUIkjdZnLxmwpNnlPc65654567 = TCIUIkjdZnLxmwpNnlPc23810886;     TCIUIkjdZnLxmwpNnlPc23810886 = TCIUIkjdZnLxmwpNnlPc37015309;     TCIUIkjdZnLxmwpNnlPc37015309 = TCIUIkjdZnLxmwpNnlPc4453468;     TCIUIkjdZnLxmwpNnlPc4453468 = TCIUIkjdZnLxmwpNnlPc40604418;     TCIUIkjdZnLxmwpNnlPc40604418 = TCIUIkjdZnLxmwpNnlPc26472964;     TCIUIkjdZnLxmwpNnlPc26472964 = TCIUIkjdZnLxmwpNnlPc55627316;     TCIUIkjdZnLxmwpNnlPc55627316 = TCIUIkjdZnLxmwpNnlPc43405054;     TCIUIkjdZnLxmwpNnlPc43405054 = TCIUIkjdZnLxmwpNnlPc57394632;     TCIUIkjdZnLxmwpNnlPc57394632 = TCIUIkjdZnLxmwpNnlPc21847805;     TCIUIkjdZnLxmwpNnlPc21847805 = TCIUIkjdZnLxmwpNnlPc26025417;     TCIUIkjdZnLxmwpNnlPc26025417 = TCIUIkjdZnLxmwpNnlPc8427232;     TCIUIkjdZnLxmwpNnlPc8427232 = TCIUIkjdZnLxmwpNnlPc80341208;     TCIUIkjdZnLxmwpNnlPc80341208 = TCIUIkjdZnLxmwpNnlPc20075376;     TCIUIkjdZnLxmwpNnlPc20075376 = TCIUIkjdZnLxmwpNnlPc75244567;     TCIUIkjdZnLxmwpNnlPc75244567 = TCIUIkjdZnLxmwpNnlPc66326394;     TCIUIkjdZnLxmwpNnlPc66326394 = TCIUIkjdZnLxmwpNnlPc2117001;     TCIUIkjdZnLxmwpNnlPc2117001 = TCIUIkjdZnLxmwpNnlPc67932758;     TCIUIkjdZnLxmwpNnlPc67932758 = TCIUIkjdZnLxmwpNnlPc16310933;     TCIUIkjdZnLxmwpNnlPc16310933 = TCIUIkjdZnLxmwpNnlPc41275244;     TCIUIkjdZnLxmwpNnlPc41275244 = TCIUIkjdZnLxmwpNnlPc12071448;     TCIUIkjdZnLxmwpNnlPc12071448 = TCIUIkjdZnLxmwpNnlPc83502816;     TCIUIkjdZnLxmwpNnlPc83502816 = TCIUIkjdZnLxmwpNnlPc36323791;     TCIUIkjdZnLxmwpNnlPc36323791 = TCIUIkjdZnLxmwpNnlPc60394797;     TCIUIkjdZnLxmwpNnlPc60394797 = TCIUIkjdZnLxmwpNnlPc14453752;     TCIUIkjdZnLxmwpNnlPc14453752 = TCIUIkjdZnLxmwpNnlPc55945096;     TCIUIkjdZnLxmwpNnlPc55945096 = TCIUIkjdZnLxmwpNnlPc42457831;     TCIUIkjdZnLxmwpNnlPc42457831 = TCIUIkjdZnLxmwpNnlPc40527484;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void fWbGsOymSLbpSKMwUuwI75091899() {     double bYdUTynBttYLyufxXTfd66112761 = -630468256;    double bYdUTynBttYLyufxXTfd81581395 = -755131696;    double bYdUTynBttYLyufxXTfd12903972 = -934520030;    double bYdUTynBttYLyufxXTfd59006181 = -630588122;    double bYdUTynBttYLyufxXTfd20166242 = -245496731;    double bYdUTynBttYLyufxXTfd4282448 = -536496314;    double bYdUTynBttYLyufxXTfd87565582 = -226916747;    double bYdUTynBttYLyufxXTfd39651475 = -206424395;    double bYdUTynBttYLyufxXTfd65395213 = -119837598;    double bYdUTynBttYLyufxXTfd64038208 = -30687184;    double bYdUTynBttYLyufxXTfd77131083 = -723084115;    double bYdUTynBttYLyufxXTfd66107805 = 93891854;    double bYdUTynBttYLyufxXTfd24979582 = -606902937;    double bYdUTynBttYLyufxXTfd87880286 = -819458531;    double bYdUTynBttYLyufxXTfd9350708 = -976490374;    double bYdUTynBttYLyufxXTfd96029937 = -906717362;    double bYdUTynBttYLyufxXTfd22438462 = -693404465;    double bYdUTynBttYLyufxXTfd1673600 = -762636771;    double bYdUTynBttYLyufxXTfd16150748 = -74182405;    double bYdUTynBttYLyufxXTfd42681705 = -667459900;    double bYdUTynBttYLyufxXTfd92957568 = -909042938;    double bYdUTynBttYLyufxXTfd84182235 = 25272033;    double bYdUTynBttYLyufxXTfd8561746 = -381928678;    double bYdUTynBttYLyufxXTfd40961744 = -482228631;    double bYdUTynBttYLyufxXTfd98463186 = -162366171;    double bYdUTynBttYLyufxXTfd44903267 = -400441246;    double bYdUTynBttYLyufxXTfd64132437 = -891790926;    double bYdUTynBttYLyufxXTfd24757977 = -998610601;    double bYdUTynBttYLyufxXTfd37980366 = -332624586;    double bYdUTynBttYLyufxXTfd29616502 = 53448481;    double bYdUTynBttYLyufxXTfd58678026 = 64178592;    double bYdUTynBttYLyufxXTfd29670556 = -283798963;    double bYdUTynBttYLyufxXTfd45368546 = -516568603;    double bYdUTynBttYLyufxXTfd75462805 = -131978507;    double bYdUTynBttYLyufxXTfd61370344 = -884100283;    double bYdUTynBttYLyufxXTfd53110914 = -440485208;    double bYdUTynBttYLyufxXTfd6733412 = -717112495;    double bYdUTynBttYLyufxXTfd86572133 = -985123748;    double bYdUTynBttYLyufxXTfd88405132 = -873484298;    double bYdUTynBttYLyufxXTfd97815132 = -895755463;    double bYdUTynBttYLyufxXTfd4860634 = -946661355;    double bYdUTynBttYLyufxXTfd98942584 = -262466862;    double bYdUTynBttYLyufxXTfd27425881 = -477302586;    double bYdUTynBttYLyufxXTfd34986809 = -942699601;    double bYdUTynBttYLyufxXTfd4755614 = -403077889;    double bYdUTynBttYLyufxXTfd54333054 = -112887954;    double bYdUTynBttYLyufxXTfd71777531 = 89605966;    double bYdUTynBttYLyufxXTfd32462667 = -894220932;    double bYdUTynBttYLyufxXTfd89822691 = -878945892;    double bYdUTynBttYLyufxXTfd57887732 = -458966294;    double bYdUTynBttYLyufxXTfd78003027 = -844012235;    double bYdUTynBttYLyufxXTfd83548762 = -167376999;    double bYdUTynBttYLyufxXTfd90478851 = -917698732;    double bYdUTynBttYLyufxXTfd17835835 = 93352800;    double bYdUTynBttYLyufxXTfd82134331 = -460711122;    double bYdUTynBttYLyufxXTfd81930526 = -555740289;    double bYdUTynBttYLyufxXTfd73019650 = -273203018;    double bYdUTynBttYLyufxXTfd71942227 = -352291399;    double bYdUTynBttYLyufxXTfd60542994 = -368221951;    double bYdUTynBttYLyufxXTfd75262975 = -845055486;    double bYdUTynBttYLyufxXTfd40150010 = -644705389;    double bYdUTynBttYLyufxXTfd62807606 = -228306147;    double bYdUTynBttYLyufxXTfd1671110 = -873799809;    double bYdUTynBttYLyufxXTfd35778711 = -73286080;    double bYdUTynBttYLyufxXTfd5360182 = 5134224;    double bYdUTynBttYLyufxXTfd47460528 = -339285152;    double bYdUTynBttYLyufxXTfd20739259 = -389539544;    double bYdUTynBttYLyufxXTfd49516776 = -374924430;    double bYdUTynBttYLyufxXTfd26509943 = -935358248;    double bYdUTynBttYLyufxXTfd56239794 = -436005166;    double bYdUTynBttYLyufxXTfd89296526 = -89604867;    double bYdUTynBttYLyufxXTfd35866329 = -708280718;    double bYdUTynBttYLyufxXTfd13268467 = -889152473;    double bYdUTynBttYLyufxXTfd18335615 = -178426942;    double bYdUTynBttYLyufxXTfd37821071 = -720798546;    double bYdUTynBttYLyufxXTfd94014983 = -546576076;    double bYdUTynBttYLyufxXTfd56756355 = -497425382;    double bYdUTynBttYLyufxXTfd73574936 = -439229078;    double bYdUTynBttYLyufxXTfd36206131 = 20849257;    double bYdUTynBttYLyufxXTfd44130133 = 50521782;    double bYdUTynBttYLyufxXTfd73125736 = -390047213;    double bYdUTynBttYLyufxXTfd31669771 = -997569994;    double bYdUTynBttYLyufxXTfd34935285 = -19664709;    double bYdUTynBttYLyufxXTfd80092633 = -873658292;    double bYdUTynBttYLyufxXTfd51613475 = -102539284;    double bYdUTynBttYLyufxXTfd75129263 = -768444409;    double bYdUTynBttYLyufxXTfd39191704 = -366100232;    double bYdUTynBttYLyufxXTfd27532711 = -509921404;    double bYdUTynBttYLyufxXTfd93328474 = -671267386;    double bYdUTynBttYLyufxXTfd79439818 = -228359995;    double bYdUTynBttYLyufxXTfd80091263 = -67282191;    double bYdUTynBttYLyufxXTfd34791185 = -264821096;    double bYdUTynBttYLyufxXTfd26029140 = -516901797;    double bYdUTynBttYLyufxXTfd13142158 = 71571188;    double bYdUTynBttYLyufxXTfd57665122 = -151050075;    double bYdUTynBttYLyufxXTfd42053027 = -618355208;    double bYdUTynBttYLyufxXTfd97271475 = -388667053;    double bYdUTynBttYLyufxXTfd91647169 = -304016507;    double bYdUTynBttYLyufxXTfd29626628 = -847833825;    double bYdUTynBttYLyufxXTfd57295086 = -630468256;     bYdUTynBttYLyufxXTfd66112761 = bYdUTynBttYLyufxXTfd81581395;     bYdUTynBttYLyufxXTfd81581395 = bYdUTynBttYLyufxXTfd12903972;     bYdUTynBttYLyufxXTfd12903972 = bYdUTynBttYLyufxXTfd59006181;     bYdUTynBttYLyufxXTfd59006181 = bYdUTynBttYLyufxXTfd20166242;     bYdUTynBttYLyufxXTfd20166242 = bYdUTynBttYLyufxXTfd4282448;     bYdUTynBttYLyufxXTfd4282448 = bYdUTynBttYLyufxXTfd87565582;     bYdUTynBttYLyufxXTfd87565582 = bYdUTynBttYLyufxXTfd39651475;     bYdUTynBttYLyufxXTfd39651475 = bYdUTynBttYLyufxXTfd65395213;     bYdUTynBttYLyufxXTfd65395213 = bYdUTynBttYLyufxXTfd64038208;     bYdUTynBttYLyufxXTfd64038208 = bYdUTynBttYLyufxXTfd77131083;     bYdUTynBttYLyufxXTfd77131083 = bYdUTynBttYLyufxXTfd66107805;     bYdUTynBttYLyufxXTfd66107805 = bYdUTynBttYLyufxXTfd24979582;     bYdUTynBttYLyufxXTfd24979582 = bYdUTynBttYLyufxXTfd87880286;     bYdUTynBttYLyufxXTfd87880286 = bYdUTynBttYLyufxXTfd9350708;     bYdUTynBttYLyufxXTfd9350708 = bYdUTynBttYLyufxXTfd96029937;     bYdUTynBttYLyufxXTfd96029937 = bYdUTynBttYLyufxXTfd22438462;     bYdUTynBttYLyufxXTfd22438462 = bYdUTynBttYLyufxXTfd1673600;     bYdUTynBttYLyufxXTfd1673600 = bYdUTynBttYLyufxXTfd16150748;     bYdUTynBttYLyufxXTfd16150748 = bYdUTynBttYLyufxXTfd42681705;     bYdUTynBttYLyufxXTfd42681705 = bYdUTynBttYLyufxXTfd92957568;     bYdUTynBttYLyufxXTfd92957568 = bYdUTynBttYLyufxXTfd84182235;     bYdUTynBttYLyufxXTfd84182235 = bYdUTynBttYLyufxXTfd8561746;     bYdUTynBttYLyufxXTfd8561746 = bYdUTynBttYLyufxXTfd40961744;     bYdUTynBttYLyufxXTfd40961744 = bYdUTynBttYLyufxXTfd98463186;     bYdUTynBttYLyufxXTfd98463186 = bYdUTynBttYLyufxXTfd44903267;     bYdUTynBttYLyufxXTfd44903267 = bYdUTynBttYLyufxXTfd64132437;     bYdUTynBttYLyufxXTfd64132437 = bYdUTynBttYLyufxXTfd24757977;     bYdUTynBttYLyufxXTfd24757977 = bYdUTynBttYLyufxXTfd37980366;     bYdUTynBttYLyufxXTfd37980366 = bYdUTynBttYLyufxXTfd29616502;     bYdUTynBttYLyufxXTfd29616502 = bYdUTynBttYLyufxXTfd58678026;     bYdUTynBttYLyufxXTfd58678026 = bYdUTynBttYLyufxXTfd29670556;     bYdUTynBttYLyufxXTfd29670556 = bYdUTynBttYLyufxXTfd45368546;     bYdUTynBttYLyufxXTfd45368546 = bYdUTynBttYLyufxXTfd75462805;     bYdUTynBttYLyufxXTfd75462805 = bYdUTynBttYLyufxXTfd61370344;     bYdUTynBttYLyufxXTfd61370344 = bYdUTynBttYLyufxXTfd53110914;     bYdUTynBttYLyufxXTfd53110914 = bYdUTynBttYLyufxXTfd6733412;     bYdUTynBttYLyufxXTfd6733412 = bYdUTynBttYLyufxXTfd86572133;     bYdUTynBttYLyufxXTfd86572133 = bYdUTynBttYLyufxXTfd88405132;     bYdUTynBttYLyufxXTfd88405132 = bYdUTynBttYLyufxXTfd97815132;     bYdUTynBttYLyufxXTfd97815132 = bYdUTynBttYLyufxXTfd4860634;     bYdUTynBttYLyufxXTfd4860634 = bYdUTynBttYLyufxXTfd98942584;     bYdUTynBttYLyufxXTfd98942584 = bYdUTynBttYLyufxXTfd27425881;     bYdUTynBttYLyufxXTfd27425881 = bYdUTynBttYLyufxXTfd34986809;     bYdUTynBttYLyufxXTfd34986809 = bYdUTynBttYLyufxXTfd4755614;     bYdUTynBttYLyufxXTfd4755614 = bYdUTynBttYLyufxXTfd54333054;     bYdUTynBttYLyufxXTfd54333054 = bYdUTynBttYLyufxXTfd71777531;     bYdUTynBttYLyufxXTfd71777531 = bYdUTynBttYLyufxXTfd32462667;     bYdUTynBttYLyufxXTfd32462667 = bYdUTynBttYLyufxXTfd89822691;     bYdUTynBttYLyufxXTfd89822691 = bYdUTynBttYLyufxXTfd57887732;     bYdUTynBttYLyufxXTfd57887732 = bYdUTynBttYLyufxXTfd78003027;     bYdUTynBttYLyufxXTfd78003027 = bYdUTynBttYLyufxXTfd83548762;     bYdUTynBttYLyufxXTfd83548762 = bYdUTynBttYLyufxXTfd90478851;     bYdUTynBttYLyufxXTfd90478851 = bYdUTynBttYLyufxXTfd17835835;     bYdUTynBttYLyufxXTfd17835835 = bYdUTynBttYLyufxXTfd82134331;     bYdUTynBttYLyufxXTfd82134331 = bYdUTynBttYLyufxXTfd81930526;     bYdUTynBttYLyufxXTfd81930526 = bYdUTynBttYLyufxXTfd73019650;     bYdUTynBttYLyufxXTfd73019650 = bYdUTynBttYLyufxXTfd71942227;     bYdUTynBttYLyufxXTfd71942227 = bYdUTynBttYLyufxXTfd60542994;     bYdUTynBttYLyufxXTfd60542994 = bYdUTynBttYLyufxXTfd75262975;     bYdUTynBttYLyufxXTfd75262975 = bYdUTynBttYLyufxXTfd40150010;     bYdUTynBttYLyufxXTfd40150010 = bYdUTynBttYLyufxXTfd62807606;     bYdUTynBttYLyufxXTfd62807606 = bYdUTynBttYLyufxXTfd1671110;     bYdUTynBttYLyufxXTfd1671110 = bYdUTynBttYLyufxXTfd35778711;     bYdUTynBttYLyufxXTfd35778711 = bYdUTynBttYLyufxXTfd5360182;     bYdUTynBttYLyufxXTfd5360182 = bYdUTynBttYLyufxXTfd47460528;     bYdUTynBttYLyufxXTfd47460528 = bYdUTynBttYLyufxXTfd20739259;     bYdUTynBttYLyufxXTfd20739259 = bYdUTynBttYLyufxXTfd49516776;     bYdUTynBttYLyufxXTfd49516776 = bYdUTynBttYLyufxXTfd26509943;     bYdUTynBttYLyufxXTfd26509943 = bYdUTynBttYLyufxXTfd56239794;     bYdUTynBttYLyufxXTfd56239794 = bYdUTynBttYLyufxXTfd89296526;     bYdUTynBttYLyufxXTfd89296526 = bYdUTynBttYLyufxXTfd35866329;     bYdUTynBttYLyufxXTfd35866329 = bYdUTynBttYLyufxXTfd13268467;     bYdUTynBttYLyufxXTfd13268467 = bYdUTynBttYLyufxXTfd18335615;     bYdUTynBttYLyufxXTfd18335615 = bYdUTynBttYLyufxXTfd37821071;     bYdUTynBttYLyufxXTfd37821071 = bYdUTynBttYLyufxXTfd94014983;     bYdUTynBttYLyufxXTfd94014983 = bYdUTynBttYLyufxXTfd56756355;     bYdUTynBttYLyufxXTfd56756355 = bYdUTynBttYLyufxXTfd73574936;     bYdUTynBttYLyufxXTfd73574936 = bYdUTynBttYLyufxXTfd36206131;     bYdUTynBttYLyufxXTfd36206131 = bYdUTynBttYLyufxXTfd44130133;     bYdUTynBttYLyufxXTfd44130133 = bYdUTynBttYLyufxXTfd73125736;     bYdUTynBttYLyufxXTfd73125736 = bYdUTynBttYLyufxXTfd31669771;     bYdUTynBttYLyufxXTfd31669771 = bYdUTynBttYLyufxXTfd34935285;     bYdUTynBttYLyufxXTfd34935285 = bYdUTynBttYLyufxXTfd80092633;     bYdUTynBttYLyufxXTfd80092633 = bYdUTynBttYLyufxXTfd51613475;     bYdUTynBttYLyufxXTfd51613475 = bYdUTynBttYLyufxXTfd75129263;     bYdUTynBttYLyufxXTfd75129263 = bYdUTynBttYLyufxXTfd39191704;     bYdUTynBttYLyufxXTfd39191704 = bYdUTynBttYLyufxXTfd27532711;     bYdUTynBttYLyufxXTfd27532711 = bYdUTynBttYLyufxXTfd93328474;     bYdUTynBttYLyufxXTfd93328474 = bYdUTynBttYLyufxXTfd79439818;     bYdUTynBttYLyufxXTfd79439818 = bYdUTynBttYLyufxXTfd80091263;     bYdUTynBttYLyufxXTfd80091263 = bYdUTynBttYLyufxXTfd34791185;     bYdUTynBttYLyufxXTfd34791185 = bYdUTynBttYLyufxXTfd26029140;     bYdUTynBttYLyufxXTfd26029140 = bYdUTynBttYLyufxXTfd13142158;     bYdUTynBttYLyufxXTfd13142158 = bYdUTynBttYLyufxXTfd57665122;     bYdUTynBttYLyufxXTfd57665122 = bYdUTynBttYLyufxXTfd42053027;     bYdUTynBttYLyufxXTfd42053027 = bYdUTynBttYLyufxXTfd97271475;     bYdUTynBttYLyufxXTfd97271475 = bYdUTynBttYLyufxXTfd91647169;     bYdUTynBttYLyufxXTfd91647169 = bYdUTynBttYLyufxXTfd29626628;     bYdUTynBttYLyufxXTfd29626628 = bYdUTynBttYLyufxXTfd57295086;     bYdUTynBttYLyufxXTfd57295086 = bYdUTynBttYLyufxXTfd66112761;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UrqfXzksxiCfHcwdFXyU2686157() {     double PEedxaZzAhYqtmNtETLY91698038 = -55501357;    double PEedxaZzAhYqtmNtETLY77978539 = 89390527;    double PEedxaZzAhYqtmNtETLY45783147 = -736981566;    double PEedxaZzAhYqtmNtETLY4728111 = -637911015;    double PEedxaZzAhYqtmNtETLY89508246 = -292419019;    double PEedxaZzAhYqtmNtETLY21291725 = -691477317;    double PEedxaZzAhYqtmNtETLY92886426 = -809187954;    double PEedxaZzAhYqtmNtETLY54661599 = -587106274;    double PEedxaZzAhYqtmNtETLY90724505 = -616061643;    double PEedxaZzAhYqtmNtETLY83196590 = -365248707;    double PEedxaZzAhYqtmNtETLY91464637 = 66487999;    double PEedxaZzAhYqtmNtETLY1425461 = -349817053;    double PEedxaZzAhYqtmNtETLY1560360 = -371151302;    double PEedxaZzAhYqtmNtETLY79440003 = -793982825;    double PEedxaZzAhYqtmNtETLY65619596 = -865036332;    double PEedxaZzAhYqtmNtETLY3750981 = -242908839;    double PEedxaZzAhYqtmNtETLY22236579 = -381724259;    double PEedxaZzAhYqtmNtETLY22789504 = -178914132;    double PEedxaZzAhYqtmNtETLY77615468 = -857156262;    double PEedxaZzAhYqtmNtETLY3843172 = -641899276;    double PEedxaZzAhYqtmNtETLY93966834 = -270742719;    double PEedxaZzAhYqtmNtETLY85685341 = -394460897;    double PEedxaZzAhYqtmNtETLY12325544 = -426020784;    double PEedxaZzAhYqtmNtETLY42675787 = -341077611;    double PEedxaZzAhYqtmNtETLY53366326 = -829862313;    double PEedxaZzAhYqtmNtETLY74392404 = 35108213;    double PEedxaZzAhYqtmNtETLY46520347 = -457604060;    double PEedxaZzAhYqtmNtETLY88656776 = -128272833;    double PEedxaZzAhYqtmNtETLY45857578 = -568918436;    double PEedxaZzAhYqtmNtETLY82938993 = -955461080;    double PEedxaZzAhYqtmNtETLY80724660 = -814468439;    double PEedxaZzAhYqtmNtETLY66860488 = -557594797;    double PEedxaZzAhYqtmNtETLY66555597 = -964389740;    double PEedxaZzAhYqtmNtETLY29049877 = -188123479;    double PEedxaZzAhYqtmNtETLY62775332 = -485536195;    double PEedxaZzAhYqtmNtETLY57902764 = -958079924;    double PEedxaZzAhYqtmNtETLY56378797 = -97624440;    double PEedxaZzAhYqtmNtETLY62144820 = -833838573;    double PEedxaZzAhYqtmNtETLY29328711 = -231822620;    double PEedxaZzAhYqtmNtETLY6598807 = -265725783;    double PEedxaZzAhYqtmNtETLY52011914 = -421721545;    double PEedxaZzAhYqtmNtETLY42952175 = -771505366;    double PEedxaZzAhYqtmNtETLY76626100 = -505908968;    double PEedxaZzAhYqtmNtETLY5780089 = -311115200;    double PEedxaZzAhYqtmNtETLY96736489 = -342099006;    double PEedxaZzAhYqtmNtETLY20733376 = -635435491;    double PEedxaZzAhYqtmNtETLY71545985 = 57137106;    double PEedxaZzAhYqtmNtETLY40575438 = 12596221;    double PEedxaZzAhYqtmNtETLY40634012 = -978622119;    double PEedxaZzAhYqtmNtETLY11697728 = -306989447;    double PEedxaZzAhYqtmNtETLY88139274 = 7329163;    double PEedxaZzAhYqtmNtETLY10807343 = -345456179;    double PEedxaZzAhYqtmNtETLY8552456 = -385597280;    double PEedxaZzAhYqtmNtETLY86734743 = -221710981;    double PEedxaZzAhYqtmNtETLY8719323 = -935801539;    double PEedxaZzAhYqtmNtETLY6012697 = -661040460;    double PEedxaZzAhYqtmNtETLY65652995 = -484588690;    double PEedxaZzAhYqtmNtETLY3107361 = -295903956;    double PEedxaZzAhYqtmNtETLY51361784 = -808048702;    double PEedxaZzAhYqtmNtETLY15115843 = -227527232;    double PEedxaZzAhYqtmNtETLY74771378 = -133873257;    double PEedxaZzAhYqtmNtETLY4229651 = -580915122;    double PEedxaZzAhYqtmNtETLY8804021 = 81812162;    double PEedxaZzAhYqtmNtETLY7785513 = -660600563;    double PEedxaZzAhYqtmNtETLY2471930 = -550780269;    double PEedxaZzAhYqtmNtETLY24604150 = -375917205;    double PEedxaZzAhYqtmNtETLY34869863 = -385427314;    double PEedxaZzAhYqtmNtETLY72510483 = -83027824;    double PEedxaZzAhYqtmNtETLY16664672 = -208446630;    double PEedxaZzAhYqtmNtETLY7716832 = -906956408;    double PEedxaZzAhYqtmNtETLY47372183 = -45284400;    double PEedxaZzAhYqtmNtETLY60091758 = -547885687;    double PEedxaZzAhYqtmNtETLY93460792 = -947091512;    double PEedxaZzAhYqtmNtETLY71016662 = -491430480;    double PEedxaZzAhYqtmNtETLY51831257 = -120177731;    double PEedxaZzAhYqtmNtETLY51014659 = -499237353;    double PEedxaZzAhYqtmNtETLY9059242 = -888551930;    double PEedxaZzAhYqtmNtETLY6545455 = -14905585;    double PEedxaZzAhYqtmNtETLY45939297 = -998978605;    double PEedxaZzAhYqtmNtETLY32632950 = -94426823;    double PEedxaZzAhYqtmNtETLY2846419 = 77971106;    double PEedxaZzAhYqtmNtETLY5944909 = -370200282;    double PEedxaZzAhYqtmNtETLY48022765 = -149650715;    double PEedxaZzAhYqtmNtETLY34159851 = -161928990;    double PEedxaZzAhYqtmNtETLY94799718 = -862790243;    double PEedxaZzAhYqtmNtETLY69917318 = -369012261;    double PEedxaZzAhYqtmNtETLY58308033 = -71997517;    double PEedxaZzAhYqtmNtETLY79820853 = -642678759;    double PEedxaZzAhYqtmNtETLY20330554 = -252321940;    double PEedxaZzAhYqtmNtETLY56762636 = -824495735;    double PEedxaZzAhYqtmNtETLY92249768 = -373491235;    double PEedxaZzAhYqtmNtETLY53271437 = -801720484;    double PEedxaZzAhYqtmNtETLY10783037 = 74210129;    double PEedxaZzAhYqtmNtETLY14212869 = 95704611;    double PEedxaZzAhYqtmNtETLY31827429 = -31852526;    double PEedxaZzAhYqtmNtETLY47782264 = -840806424;    double PEedxaZzAhYqtmNtETLY34148154 = -753317529;    double PEedxaZzAhYqtmNtETLY68840587 = -845308405;    double PEedxaZzAhYqtmNtETLY3308159 = -760334931;    double PEedxaZzAhYqtmNtETLY72132340 = -55501357;     PEedxaZzAhYqtmNtETLY91698038 = PEedxaZzAhYqtmNtETLY77978539;     PEedxaZzAhYqtmNtETLY77978539 = PEedxaZzAhYqtmNtETLY45783147;     PEedxaZzAhYqtmNtETLY45783147 = PEedxaZzAhYqtmNtETLY4728111;     PEedxaZzAhYqtmNtETLY4728111 = PEedxaZzAhYqtmNtETLY89508246;     PEedxaZzAhYqtmNtETLY89508246 = PEedxaZzAhYqtmNtETLY21291725;     PEedxaZzAhYqtmNtETLY21291725 = PEedxaZzAhYqtmNtETLY92886426;     PEedxaZzAhYqtmNtETLY92886426 = PEedxaZzAhYqtmNtETLY54661599;     PEedxaZzAhYqtmNtETLY54661599 = PEedxaZzAhYqtmNtETLY90724505;     PEedxaZzAhYqtmNtETLY90724505 = PEedxaZzAhYqtmNtETLY83196590;     PEedxaZzAhYqtmNtETLY83196590 = PEedxaZzAhYqtmNtETLY91464637;     PEedxaZzAhYqtmNtETLY91464637 = PEedxaZzAhYqtmNtETLY1425461;     PEedxaZzAhYqtmNtETLY1425461 = PEedxaZzAhYqtmNtETLY1560360;     PEedxaZzAhYqtmNtETLY1560360 = PEedxaZzAhYqtmNtETLY79440003;     PEedxaZzAhYqtmNtETLY79440003 = PEedxaZzAhYqtmNtETLY65619596;     PEedxaZzAhYqtmNtETLY65619596 = PEedxaZzAhYqtmNtETLY3750981;     PEedxaZzAhYqtmNtETLY3750981 = PEedxaZzAhYqtmNtETLY22236579;     PEedxaZzAhYqtmNtETLY22236579 = PEedxaZzAhYqtmNtETLY22789504;     PEedxaZzAhYqtmNtETLY22789504 = PEedxaZzAhYqtmNtETLY77615468;     PEedxaZzAhYqtmNtETLY77615468 = PEedxaZzAhYqtmNtETLY3843172;     PEedxaZzAhYqtmNtETLY3843172 = PEedxaZzAhYqtmNtETLY93966834;     PEedxaZzAhYqtmNtETLY93966834 = PEedxaZzAhYqtmNtETLY85685341;     PEedxaZzAhYqtmNtETLY85685341 = PEedxaZzAhYqtmNtETLY12325544;     PEedxaZzAhYqtmNtETLY12325544 = PEedxaZzAhYqtmNtETLY42675787;     PEedxaZzAhYqtmNtETLY42675787 = PEedxaZzAhYqtmNtETLY53366326;     PEedxaZzAhYqtmNtETLY53366326 = PEedxaZzAhYqtmNtETLY74392404;     PEedxaZzAhYqtmNtETLY74392404 = PEedxaZzAhYqtmNtETLY46520347;     PEedxaZzAhYqtmNtETLY46520347 = PEedxaZzAhYqtmNtETLY88656776;     PEedxaZzAhYqtmNtETLY88656776 = PEedxaZzAhYqtmNtETLY45857578;     PEedxaZzAhYqtmNtETLY45857578 = PEedxaZzAhYqtmNtETLY82938993;     PEedxaZzAhYqtmNtETLY82938993 = PEedxaZzAhYqtmNtETLY80724660;     PEedxaZzAhYqtmNtETLY80724660 = PEedxaZzAhYqtmNtETLY66860488;     PEedxaZzAhYqtmNtETLY66860488 = PEedxaZzAhYqtmNtETLY66555597;     PEedxaZzAhYqtmNtETLY66555597 = PEedxaZzAhYqtmNtETLY29049877;     PEedxaZzAhYqtmNtETLY29049877 = PEedxaZzAhYqtmNtETLY62775332;     PEedxaZzAhYqtmNtETLY62775332 = PEedxaZzAhYqtmNtETLY57902764;     PEedxaZzAhYqtmNtETLY57902764 = PEedxaZzAhYqtmNtETLY56378797;     PEedxaZzAhYqtmNtETLY56378797 = PEedxaZzAhYqtmNtETLY62144820;     PEedxaZzAhYqtmNtETLY62144820 = PEedxaZzAhYqtmNtETLY29328711;     PEedxaZzAhYqtmNtETLY29328711 = PEedxaZzAhYqtmNtETLY6598807;     PEedxaZzAhYqtmNtETLY6598807 = PEedxaZzAhYqtmNtETLY52011914;     PEedxaZzAhYqtmNtETLY52011914 = PEedxaZzAhYqtmNtETLY42952175;     PEedxaZzAhYqtmNtETLY42952175 = PEedxaZzAhYqtmNtETLY76626100;     PEedxaZzAhYqtmNtETLY76626100 = PEedxaZzAhYqtmNtETLY5780089;     PEedxaZzAhYqtmNtETLY5780089 = PEedxaZzAhYqtmNtETLY96736489;     PEedxaZzAhYqtmNtETLY96736489 = PEedxaZzAhYqtmNtETLY20733376;     PEedxaZzAhYqtmNtETLY20733376 = PEedxaZzAhYqtmNtETLY71545985;     PEedxaZzAhYqtmNtETLY71545985 = PEedxaZzAhYqtmNtETLY40575438;     PEedxaZzAhYqtmNtETLY40575438 = PEedxaZzAhYqtmNtETLY40634012;     PEedxaZzAhYqtmNtETLY40634012 = PEedxaZzAhYqtmNtETLY11697728;     PEedxaZzAhYqtmNtETLY11697728 = PEedxaZzAhYqtmNtETLY88139274;     PEedxaZzAhYqtmNtETLY88139274 = PEedxaZzAhYqtmNtETLY10807343;     PEedxaZzAhYqtmNtETLY10807343 = PEedxaZzAhYqtmNtETLY8552456;     PEedxaZzAhYqtmNtETLY8552456 = PEedxaZzAhYqtmNtETLY86734743;     PEedxaZzAhYqtmNtETLY86734743 = PEedxaZzAhYqtmNtETLY8719323;     PEedxaZzAhYqtmNtETLY8719323 = PEedxaZzAhYqtmNtETLY6012697;     PEedxaZzAhYqtmNtETLY6012697 = PEedxaZzAhYqtmNtETLY65652995;     PEedxaZzAhYqtmNtETLY65652995 = PEedxaZzAhYqtmNtETLY3107361;     PEedxaZzAhYqtmNtETLY3107361 = PEedxaZzAhYqtmNtETLY51361784;     PEedxaZzAhYqtmNtETLY51361784 = PEedxaZzAhYqtmNtETLY15115843;     PEedxaZzAhYqtmNtETLY15115843 = PEedxaZzAhYqtmNtETLY74771378;     PEedxaZzAhYqtmNtETLY74771378 = PEedxaZzAhYqtmNtETLY4229651;     PEedxaZzAhYqtmNtETLY4229651 = PEedxaZzAhYqtmNtETLY8804021;     PEedxaZzAhYqtmNtETLY8804021 = PEedxaZzAhYqtmNtETLY7785513;     PEedxaZzAhYqtmNtETLY7785513 = PEedxaZzAhYqtmNtETLY2471930;     PEedxaZzAhYqtmNtETLY2471930 = PEedxaZzAhYqtmNtETLY24604150;     PEedxaZzAhYqtmNtETLY24604150 = PEedxaZzAhYqtmNtETLY34869863;     PEedxaZzAhYqtmNtETLY34869863 = PEedxaZzAhYqtmNtETLY72510483;     PEedxaZzAhYqtmNtETLY72510483 = PEedxaZzAhYqtmNtETLY16664672;     PEedxaZzAhYqtmNtETLY16664672 = PEedxaZzAhYqtmNtETLY7716832;     PEedxaZzAhYqtmNtETLY7716832 = PEedxaZzAhYqtmNtETLY47372183;     PEedxaZzAhYqtmNtETLY47372183 = PEedxaZzAhYqtmNtETLY60091758;     PEedxaZzAhYqtmNtETLY60091758 = PEedxaZzAhYqtmNtETLY93460792;     PEedxaZzAhYqtmNtETLY93460792 = PEedxaZzAhYqtmNtETLY71016662;     PEedxaZzAhYqtmNtETLY71016662 = PEedxaZzAhYqtmNtETLY51831257;     PEedxaZzAhYqtmNtETLY51831257 = PEedxaZzAhYqtmNtETLY51014659;     PEedxaZzAhYqtmNtETLY51014659 = PEedxaZzAhYqtmNtETLY9059242;     PEedxaZzAhYqtmNtETLY9059242 = PEedxaZzAhYqtmNtETLY6545455;     PEedxaZzAhYqtmNtETLY6545455 = PEedxaZzAhYqtmNtETLY45939297;     PEedxaZzAhYqtmNtETLY45939297 = PEedxaZzAhYqtmNtETLY32632950;     PEedxaZzAhYqtmNtETLY32632950 = PEedxaZzAhYqtmNtETLY2846419;     PEedxaZzAhYqtmNtETLY2846419 = PEedxaZzAhYqtmNtETLY5944909;     PEedxaZzAhYqtmNtETLY5944909 = PEedxaZzAhYqtmNtETLY48022765;     PEedxaZzAhYqtmNtETLY48022765 = PEedxaZzAhYqtmNtETLY34159851;     PEedxaZzAhYqtmNtETLY34159851 = PEedxaZzAhYqtmNtETLY94799718;     PEedxaZzAhYqtmNtETLY94799718 = PEedxaZzAhYqtmNtETLY69917318;     PEedxaZzAhYqtmNtETLY69917318 = PEedxaZzAhYqtmNtETLY58308033;     PEedxaZzAhYqtmNtETLY58308033 = PEedxaZzAhYqtmNtETLY79820853;     PEedxaZzAhYqtmNtETLY79820853 = PEedxaZzAhYqtmNtETLY20330554;     PEedxaZzAhYqtmNtETLY20330554 = PEedxaZzAhYqtmNtETLY56762636;     PEedxaZzAhYqtmNtETLY56762636 = PEedxaZzAhYqtmNtETLY92249768;     PEedxaZzAhYqtmNtETLY92249768 = PEedxaZzAhYqtmNtETLY53271437;     PEedxaZzAhYqtmNtETLY53271437 = PEedxaZzAhYqtmNtETLY10783037;     PEedxaZzAhYqtmNtETLY10783037 = PEedxaZzAhYqtmNtETLY14212869;     PEedxaZzAhYqtmNtETLY14212869 = PEedxaZzAhYqtmNtETLY31827429;     PEedxaZzAhYqtmNtETLY31827429 = PEedxaZzAhYqtmNtETLY47782264;     PEedxaZzAhYqtmNtETLY47782264 = PEedxaZzAhYqtmNtETLY34148154;     PEedxaZzAhYqtmNtETLY34148154 = PEedxaZzAhYqtmNtETLY68840587;     PEedxaZzAhYqtmNtETLY68840587 = PEedxaZzAhYqtmNtETLY3308159;     PEedxaZzAhYqtmNtETLY3308159 = PEedxaZzAhYqtmNtETLY72132340;     PEedxaZzAhYqtmNtETLY72132340 = PEedxaZzAhYqtmNtETLY91698038;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VaCVxCSGEQcPWDwJGQSQ85679389() {     double OKSzeIgQEHxGgmDIxZuI96350818 = -736159976;    double OKSzeIgQEHxGgmDIxZuI59619237 = -500823068;    double OKSzeIgQEHxGgmDIxZuI81844096 = -836648333;    double OKSzeIgQEHxGgmDIxZuI72433017 = -874148634;    double OKSzeIgQEHxGgmDIxZuI66169943 = -631943088;    double OKSzeIgQEHxGgmDIxZuI44167480 = -745486356;    double OKSzeIgQEHxGgmDIxZuI56123965 = -762214744;    double OKSzeIgQEHxGgmDIxZuI51140216 = -463718836;    double OKSzeIgQEHxGgmDIxZuI49183957 = -866326531;    double OKSzeIgQEHxGgmDIxZuI64988454 = -278251694;    double OKSzeIgQEHxGgmDIxZuI75156083 = 1263408;    double OKSzeIgQEHxGgmDIxZuI67763093 = 79889267;    double OKSzeIgQEHxGgmDIxZuI23340903 = 94893775;    double OKSzeIgQEHxGgmDIxZuI60867489 = 57625334;    double OKSzeIgQEHxGgmDIxZuI57136696 = -762836176;    double OKSzeIgQEHxGgmDIxZuI87159653 = -231267316;    double OKSzeIgQEHxGgmDIxZuI36031532 = -260227730;    double OKSzeIgQEHxGgmDIxZuI71136224 = -66746504;    double OKSzeIgQEHxGgmDIxZuI58943136 = -747463377;    double OKSzeIgQEHxGgmDIxZuI89496167 = -590271535;    double OKSzeIgQEHxGgmDIxZuI48058578 = -216442463;    double OKSzeIgQEHxGgmDIxZuI81511997 = -413402976;    double OKSzeIgQEHxGgmDIxZuI58014974 = -852503665;    double OKSzeIgQEHxGgmDIxZuI5016683 = -560348558;    double OKSzeIgQEHxGgmDIxZuI31229615 = -484449229;    double OKSzeIgQEHxGgmDIxZuI44776870 = -495652054;    double OKSzeIgQEHxGgmDIxZuI41065666 = 12351733;    double OKSzeIgQEHxGgmDIxZuI50756658 = -838566441;    double OKSzeIgQEHxGgmDIxZuI38924867 = -835680890;    double OKSzeIgQEHxGgmDIxZuI71396869 = -65076892;    double OKSzeIgQEHxGgmDIxZuI40783359 = -910114274;    double OKSzeIgQEHxGgmDIxZuI94033062 = -517046765;    double OKSzeIgQEHxGgmDIxZuI5574580 = -196393408;    double OKSzeIgQEHxGgmDIxZuI23776479 = -662414721;    double OKSzeIgQEHxGgmDIxZuI67868998 = -768227935;    double OKSzeIgQEHxGgmDIxZuI55936354 = -401016958;    double OKSzeIgQEHxGgmDIxZuI9068627 = -797964405;    double OKSzeIgQEHxGgmDIxZuI42601479 = -706583264;    double OKSzeIgQEHxGgmDIxZuI38926759 = 33058425;    double OKSzeIgQEHxGgmDIxZuI23553425 = -651392303;    double OKSzeIgQEHxGgmDIxZuI4935233 = -281257678;    double OKSzeIgQEHxGgmDIxZuI76051248 = -327485473;    double OKSzeIgQEHxGgmDIxZuI57697123 = 63803151;    double OKSzeIgQEHxGgmDIxZuI54582462 = -774035976;    double OKSzeIgQEHxGgmDIxZuI27725066 = -317931454;    double OKSzeIgQEHxGgmDIxZuI86173970 = -382236274;    double OKSzeIgQEHxGgmDIxZuI44944145 = -640240433;    double OKSzeIgQEHxGgmDIxZuI7848643 = -942246490;    double OKSzeIgQEHxGgmDIxZuI12641376 = 55706727;    double OKSzeIgQEHxGgmDIxZuI11550746 = -443764963;    double OKSzeIgQEHxGgmDIxZuI16567657 = -644525091;    double OKSzeIgQEHxGgmDIxZuI53826306 = -131058599;    double OKSzeIgQEHxGgmDIxZuI84142545 = -373526240;    double OKSzeIgQEHxGgmDIxZuI96713068 = 24422571;    double OKSzeIgQEHxGgmDIxZuI37620812 = -170935040;    double OKSzeIgQEHxGgmDIxZuI14838821 = -222757001;    double OKSzeIgQEHxGgmDIxZuI1604264 = -648319404;    double OKSzeIgQEHxGgmDIxZuI76827414 = -176299776;    double OKSzeIgQEHxGgmDIxZuI41203402 = -289699406;    double OKSzeIgQEHxGgmDIxZuI21393073 = -36291035;    double OKSzeIgQEHxGgmDIxZuI3101814 = -657838089;    double OKSzeIgQEHxGgmDIxZuI5367307 = -923648303;    double OKSzeIgQEHxGgmDIxZuI12215349 = -628037946;    double OKSzeIgQEHxGgmDIxZuI77787087 = -701249639;    double OKSzeIgQEHxGgmDIxZuI24205095 = -368137421;    double OKSzeIgQEHxGgmDIxZuI81123021 = -481689827;    double OKSzeIgQEHxGgmDIxZuI62188513 = -723717325;    double OKSzeIgQEHxGgmDIxZuI99564424 = -242691504;    double OKSzeIgQEHxGgmDIxZuI92998490 = -174146731;    double OKSzeIgQEHxGgmDIxZuI1200343 = -261819219;    double OKSzeIgQEHxGgmDIxZuI78091026 = -433302912;    double OKSzeIgQEHxGgmDIxZuI93430052 = -553644467;    double OKSzeIgQEHxGgmDIxZuI32209465 = 195071;    double OKSzeIgQEHxGgmDIxZuI35389712 = 3928926;    double OKSzeIgQEHxGgmDIxZuI84560935 = -209013858;    double OKSzeIgQEHxGgmDIxZuI72007329 = -888956990;    double OKSzeIgQEHxGgmDIxZuI23814875 = -377206127;    double OKSzeIgQEHxGgmDIxZuI3432512 = 21532310;    double OKSzeIgQEHxGgmDIxZuI77291616 = -142417104;    double OKSzeIgQEHxGgmDIxZuI45055645 = -2212956;    double OKSzeIgQEHxGgmDIxZuI99832725 = -855411621;    double OKSzeIgQEHxGgmDIxZuI33217024 = -45401778;    double OKSzeIgQEHxGgmDIxZuI38115282 = -794273168;    double OKSzeIgQEHxGgmDIxZuI27374122 = -291915928;    double OKSzeIgQEHxGgmDIxZuI54829213 = -420551801;    double OKSzeIgQEHxGgmDIxZuI86957052 = -679055675;    double OKSzeIgQEHxGgmDIxZuI9890518 = -43520526;    double OKSzeIgQEHxGgmDIxZuI8861511 = -120815980;    double OKSzeIgQEHxGgmDIxZuI86155666 = -391479682;    double OKSzeIgQEHxGgmDIxZuI53030178 = -445470935;    double OKSzeIgQEHxGgmDIxZuI54332090 = -752697555;    double OKSzeIgQEHxGgmDIxZuI32241213 = -521664630;    double OKSzeIgQEHxGgmDIxZuI1398077 = -316883859;    double OKSzeIgQEHxGgmDIxZuI17533687 = -930650541;    double OKSzeIgQEHxGgmDIxZuI20451611 = -993554215;    double OKSzeIgQEHxGgmDIxZuI99567925 = -357609375;    double OKSzeIgQEHxGgmDIxZuI63835900 = -699447528;    double OKSzeIgQEHxGgmDIxZuI79910035 = -234947210;    double OKSzeIgQEHxGgmDIxZuI30377367 = -305898555;    double OKSzeIgQEHxGgmDIxZuI46602045 = -736159976;     OKSzeIgQEHxGgmDIxZuI96350818 = OKSzeIgQEHxGgmDIxZuI59619237;     OKSzeIgQEHxGgmDIxZuI59619237 = OKSzeIgQEHxGgmDIxZuI81844096;     OKSzeIgQEHxGgmDIxZuI81844096 = OKSzeIgQEHxGgmDIxZuI72433017;     OKSzeIgQEHxGgmDIxZuI72433017 = OKSzeIgQEHxGgmDIxZuI66169943;     OKSzeIgQEHxGgmDIxZuI66169943 = OKSzeIgQEHxGgmDIxZuI44167480;     OKSzeIgQEHxGgmDIxZuI44167480 = OKSzeIgQEHxGgmDIxZuI56123965;     OKSzeIgQEHxGgmDIxZuI56123965 = OKSzeIgQEHxGgmDIxZuI51140216;     OKSzeIgQEHxGgmDIxZuI51140216 = OKSzeIgQEHxGgmDIxZuI49183957;     OKSzeIgQEHxGgmDIxZuI49183957 = OKSzeIgQEHxGgmDIxZuI64988454;     OKSzeIgQEHxGgmDIxZuI64988454 = OKSzeIgQEHxGgmDIxZuI75156083;     OKSzeIgQEHxGgmDIxZuI75156083 = OKSzeIgQEHxGgmDIxZuI67763093;     OKSzeIgQEHxGgmDIxZuI67763093 = OKSzeIgQEHxGgmDIxZuI23340903;     OKSzeIgQEHxGgmDIxZuI23340903 = OKSzeIgQEHxGgmDIxZuI60867489;     OKSzeIgQEHxGgmDIxZuI60867489 = OKSzeIgQEHxGgmDIxZuI57136696;     OKSzeIgQEHxGgmDIxZuI57136696 = OKSzeIgQEHxGgmDIxZuI87159653;     OKSzeIgQEHxGgmDIxZuI87159653 = OKSzeIgQEHxGgmDIxZuI36031532;     OKSzeIgQEHxGgmDIxZuI36031532 = OKSzeIgQEHxGgmDIxZuI71136224;     OKSzeIgQEHxGgmDIxZuI71136224 = OKSzeIgQEHxGgmDIxZuI58943136;     OKSzeIgQEHxGgmDIxZuI58943136 = OKSzeIgQEHxGgmDIxZuI89496167;     OKSzeIgQEHxGgmDIxZuI89496167 = OKSzeIgQEHxGgmDIxZuI48058578;     OKSzeIgQEHxGgmDIxZuI48058578 = OKSzeIgQEHxGgmDIxZuI81511997;     OKSzeIgQEHxGgmDIxZuI81511997 = OKSzeIgQEHxGgmDIxZuI58014974;     OKSzeIgQEHxGgmDIxZuI58014974 = OKSzeIgQEHxGgmDIxZuI5016683;     OKSzeIgQEHxGgmDIxZuI5016683 = OKSzeIgQEHxGgmDIxZuI31229615;     OKSzeIgQEHxGgmDIxZuI31229615 = OKSzeIgQEHxGgmDIxZuI44776870;     OKSzeIgQEHxGgmDIxZuI44776870 = OKSzeIgQEHxGgmDIxZuI41065666;     OKSzeIgQEHxGgmDIxZuI41065666 = OKSzeIgQEHxGgmDIxZuI50756658;     OKSzeIgQEHxGgmDIxZuI50756658 = OKSzeIgQEHxGgmDIxZuI38924867;     OKSzeIgQEHxGgmDIxZuI38924867 = OKSzeIgQEHxGgmDIxZuI71396869;     OKSzeIgQEHxGgmDIxZuI71396869 = OKSzeIgQEHxGgmDIxZuI40783359;     OKSzeIgQEHxGgmDIxZuI40783359 = OKSzeIgQEHxGgmDIxZuI94033062;     OKSzeIgQEHxGgmDIxZuI94033062 = OKSzeIgQEHxGgmDIxZuI5574580;     OKSzeIgQEHxGgmDIxZuI5574580 = OKSzeIgQEHxGgmDIxZuI23776479;     OKSzeIgQEHxGgmDIxZuI23776479 = OKSzeIgQEHxGgmDIxZuI67868998;     OKSzeIgQEHxGgmDIxZuI67868998 = OKSzeIgQEHxGgmDIxZuI55936354;     OKSzeIgQEHxGgmDIxZuI55936354 = OKSzeIgQEHxGgmDIxZuI9068627;     OKSzeIgQEHxGgmDIxZuI9068627 = OKSzeIgQEHxGgmDIxZuI42601479;     OKSzeIgQEHxGgmDIxZuI42601479 = OKSzeIgQEHxGgmDIxZuI38926759;     OKSzeIgQEHxGgmDIxZuI38926759 = OKSzeIgQEHxGgmDIxZuI23553425;     OKSzeIgQEHxGgmDIxZuI23553425 = OKSzeIgQEHxGgmDIxZuI4935233;     OKSzeIgQEHxGgmDIxZuI4935233 = OKSzeIgQEHxGgmDIxZuI76051248;     OKSzeIgQEHxGgmDIxZuI76051248 = OKSzeIgQEHxGgmDIxZuI57697123;     OKSzeIgQEHxGgmDIxZuI57697123 = OKSzeIgQEHxGgmDIxZuI54582462;     OKSzeIgQEHxGgmDIxZuI54582462 = OKSzeIgQEHxGgmDIxZuI27725066;     OKSzeIgQEHxGgmDIxZuI27725066 = OKSzeIgQEHxGgmDIxZuI86173970;     OKSzeIgQEHxGgmDIxZuI86173970 = OKSzeIgQEHxGgmDIxZuI44944145;     OKSzeIgQEHxGgmDIxZuI44944145 = OKSzeIgQEHxGgmDIxZuI7848643;     OKSzeIgQEHxGgmDIxZuI7848643 = OKSzeIgQEHxGgmDIxZuI12641376;     OKSzeIgQEHxGgmDIxZuI12641376 = OKSzeIgQEHxGgmDIxZuI11550746;     OKSzeIgQEHxGgmDIxZuI11550746 = OKSzeIgQEHxGgmDIxZuI16567657;     OKSzeIgQEHxGgmDIxZuI16567657 = OKSzeIgQEHxGgmDIxZuI53826306;     OKSzeIgQEHxGgmDIxZuI53826306 = OKSzeIgQEHxGgmDIxZuI84142545;     OKSzeIgQEHxGgmDIxZuI84142545 = OKSzeIgQEHxGgmDIxZuI96713068;     OKSzeIgQEHxGgmDIxZuI96713068 = OKSzeIgQEHxGgmDIxZuI37620812;     OKSzeIgQEHxGgmDIxZuI37620812 = OKSzeIgQEHxGgmDIxZuI14838821;     OKSzeIgQEHxGgmDIxZuI14838821 = OKSzeIgQEHxGgmDIxZuI1604264;     OKSzeIgQEHxGgmDIxZuI1604264 = OKSzeIgQEHxGgmDIxZuI76827414;     OKSzeIgQEHxGgmDIxZuI76827414 = OKSzeIgQEHxGgmDIxZuI41203402;     OKSzeIgQEHxGgmDIxZuI41203402 = OKSzeIgQEHxGgmDIxZuI21393073;     OKSzeIgQEHxGgmDIxZuI21393073 = OKSzeIgQEHxGgmDIxZuI3101814;     OKSzeIgQEHxGgmDIxZuI3101814 = OKSzeIgQEHxGgmDIxZuI5367307;     OKSzeIgQEHxGgmDIxZuI5367307 = OKSzeIgQEHxGgmDIxZuI12215349;     OKSzeIgQEHxGgmDIxZuI12215349 = OKSzeIgQEHxGgmDIxZuI77787087;     OKSzeIgQEHxGgmDIxZuI77787087 = OKSzeIgQEHxGgmDIxZuI24205095;     OKSzeIgQEHxGgmDIxZuI24205095 = OKSzeIgQEHxGgmDIxZuI81123021;     OKSzeIgQEHxGgmDIxZuI81123021 = OKSzeIgQEHxGgmDIxZuI62188513;     OKSzeIgQEHxGgmDIxZuI62188513 = OKSzeIgQEHxGgmDIxZuI99564424;     OKSzeIgQEHxGgmDIxZuI99564424 = OKSzeIgQEHxGgmDIxZuI92998490;     OKSzeIgQEHxGgmDIxZuI92998490 = OKSzeIgQEHxGgmDIxZuI1200343;     OKSzeIgQEHxGgmDIxZuI1200343 = OKSzeIgQEHxGgmDIxZuI78091026;     OKSzeIgQEHxGgmDIxZuI78091026 = OKSzeIgQEHxGgmDIxZuI93430052;     OKSzeIgQEHxGgmDIxZuI93430052 = OKSzeIgQEHxGgmDIxZuI32209465;     OKSzeIgQEHxGgmDIxZuI32209465 = OKSzeIgQEHxGgmDIxZuI35389712;     OKSzeIgQEHxGgmDIxZuI35389712 = OKSzeIgQEHxGgmDIxZuI84560935;     OKSzeIgQEHxGgmDIxZuI84560935 = OKSzeIgQEHxGgmDIxZuI72007329;     OKSzeIgQEHxGgmDIxZuI72007329 = OKSzeIgQEHxGgmDIxZuI23814875;     OKSzeIgQEHxGgmDIxZuI23814875 = OKSzeIgQEHxGgmDIxZuI3432512;     OKSzeIgQEHxGgmDIxZuI3432512 = OKSzeIgQEHxGgmDIxZuI77291616;     OKSzeIgQEHxGgmDIxZuI77291616 = OKSzeIgQEHxGgmDIxZuI45055645;     OKSzeIgQEHxGgmDIxZuI45055645 = OKSzeIgQEHxGgmDIxZuI99832725;     OKSzeIgQEHxGgmDIxZuI99832725 = OKSzeIgQEHxGgmDIxZuI33217024;     OKSzeIgQEHxGgmDIxZuI33217024 = OKSzeIgQEHxGgmDIxZuI38115282;     OKSzeIgQEHxGgmDIxZuI38115282 = OKSzeIgQEHxGgmDIxZuI27374122;     OKSzeIgQEHxGgmDIxZuI27374122 = OKSzeIgQEHxGgmDIxZuI54829213;     OKSzeIgQEHxGgmDIxZuI54829213 = OKSzeIgQEHxGgmDIxZuI86957052;     OKSzeIgQEHxGgmDIxZuI86957052 = OKSzeIgQEHxGgmDIxZuI9890518;     OKSzeIgQEHxGgmDIxZuI9890518 = OKSzeIgQEHxGgmDIxZuI8861511;     OKSzeIgQEHxGgmDIxZuI8861511 = OKSzeIgQEHxGgmDIxZuI86155666;     OKSzeIgQEHxGgmDIxZuI86155666 = OKSzeIgQEHxGgmDIxZuI53030178;     OKSzeIgQEHxGgmDIxZuI53030178 = OKSzeIgQEHxGgmDIxZuI54332090;     OKSzeIgQEHxGgmDIxZuI54332090 = OKSzeIgQEHxGgmDIxZuI32241213;     OKSzeIgQEHxGgmDIxZuI32241213 = OKSzeIgQEHxGgmDIxZuI1398077;     OKSzeIgQEHxGgmDIxZuI1398077 = OKSzeIgQEHxGgmDIxZuI17533687;     OKSzeIgQEHxGgmDIxZuI17533687 = OKSzeIgQEHxGgmDIxZuI20451611;     OKSzeIgQEHxGgmDIxZuI20451611 = OKSzeIgQEHxGgmDIxZuI99567925;     OKSzeIgQEHxGgmDIxZuI99567925 = OKSzeIgQEHxGgmDIxZuI63835900;     OKSzeIgQEHxGgmDIxZuI63835900 = OKSzeIgQEHxGgmDIxZuI79910035;     OKSzeIgQEHxGgmDIxZuI79910035 = OKSzeIgQEHxGgmDIxZuI30377367;     OKSzeIgQEHxGgmDIxZuI30377367 = OKSzeIgQEHxGgmDIxZuI46602045;     OKSzeIgQEHxGgmDIxZuI46602045 = OKSzeIgQEHxGgmDIxZuI96350818;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GGepKJIvXDDflHuyRxNb57874670() {     double NlsdldfwnTUrrUNEgmxa42868592 = -5567558;    double NlsdldfwnTUrrUNEgmxa70772825 = -421565027;    double NlsdldfwnTUrrUNEgmxa11541498 = -341904638;    double NlsdldfwnTUrrUNEgmxa96171969 = -652556802;    double NlsdldfwnTUrrUNEgmxa28192257 = -386263593;    double NlsdldfwnTUrrUNEgmxa55310277 = 98560676;    double NlsdldfwnTUrrUNEgmxa3528116 = -873730368;    double NlsdldfwnTUrrUNEgmxa84681848 = -248470032;    double NlsdldfwnTUrrUNEgmxa41383091 = -508509732;    double NlsdldfwnTUrrUNEgmxa21513355 = 65628246;    double NlsdldfwnTUrrUNEgmxa20131745 = -554367775;    double NlsdldfwnTUrrUNEgmxa72060772 = -137234866;    double NlsdldfwnTUrrUNEgmxa54721915 = -999648033;    double NlsdldfwnTUrrUNEgmxa62559437 = -743031412;    double NlsdldfwnTUrrUNEgmxa78157371 = -642128247;    double NlsdldfwnTUrrUNEgmxa19193067 = -15291793;    double NlsdldfwnTUrrUNEgmxa21832811 = -858363848;    double NlsdldfwnTUrrUNEgmxa65021312 = -111468853;    double NlsdldfwnTUrrUNEgmxa544911 = -223103977;    double NlsdldfwnTUrrUNEgmxa26166105 = -590778027;    double NlsdldfwnTUrrUNEgmxa95985364 = -94142281;    double NlsdldfwnTUrrUNEgmxa88691555 = -133926756;    double NlsdldfwnTUrrUNEgmxa19853139 = -514204994;    double NlsdldfwnTUrrUNEgmxa46103871 = -58775570;    double NlsdldfwnTUrrUNEgmxa63172605 = 35145402;    double NlsdldfwnTUrrUNEgmxa33370677 = -193792871;    double NlsdldfwnTUrrUNEgmxa11296165 = -689230330;    double NlsdldfwnTUrrUNEgmxa16454377 = -587597298;    double NlsdldfwnTUrrUNEgmxa61612003 = 58493863;    double NlsdldfwnTUrrUNEgmxa89583974 = -773280204;    double NlsdldfwnTUrrUNEgmxa24817930 = -371762501;    double NlsdldfwnTUrrUNEgmxa41240353 = -5186464;    double NlsdldfwnTUrrUNEgmxa8929702 = -760032014;    double NlsdldfwnTUrrUNEgmxa36224019 = -300413422;    double NlsdldfwnTUrrUNEgmxa65585308 = -788408018;    double NlsdldfwnTUrrUNEgmxa67486465 = -893269356;    double NlsdldfwnTUrrUNEgmxa55669568 = 41351671;    double NlsdldfwnTUrrUNEgmxa13290194 = -531268223;    double NlsdldfwnTUrrUNEgmxa11175869 = -48499264;    double NlsdldfwnTUrrUNEgmxa24166154 = -105666421;    double NlsdldfwnTUrrUNEgmxa46314476 = -471841924;    double NlsdldfwnTUrrUNEgmxa30971355 = -689582374;    double NlsdldfwnTUrrUNEgmxa75026538 = -563121730;    double NlsdldfwnTUrrUNEgmxa47366647 = -147946397;    double NlsdldfwnTUrrUNEgmxa80698242 = -220141241;    double NlsdldfwnTUrrUNEgmxa53534020 = -580530566;    double NlsdldfwnTUrrUNEgmxa71082893 = -7800615;    double NlsdldfwnTUrrUNEgmxa56800980 = -373769472;    double NlsdldfwnTUrrUNEgmxa42256652 = -77974572;    double NlsdldfwnTUrrUNEgmxa19317718 = -3035753;    double NlsdldfwnTUrrUNEgmxa8411770 = -489988042;    double NlsdldfwnTUrrUNEgmxa65324501 = -701614538;    double NlsdldfwnTUrrUNEgmxa44699662 = -421394378;    double NlsdldfwnTUrrUNEgmxa24532562 = -851838544;    double NlsdldfwnTUrrUNEgmxa61889305 = -785982374;    double NlsdldfwnTUrrUNEgmxa54177037 = -871640803;    double NlsdldfwnTUrrUNEgmxa50919686 = -907360033;    double NlsdldfwnTUrrUNEgmxa65437626 = -183129069;    double NlsdldfwnTUrrUNEgmxa32999365 = -587702204;    double NlsdldfwnTUrrUNEgmxa94821579 = -92470723;    double NlsdldfwnTUrrUNEgmxa44014113 = -212208995;    double NlsdldfwnTUrrUNEgmxa87073739 = -186133070;    double NlsdldfwnTUrrUNEgmxa23069845 = -206963895;    double NlsdldfwnTUrrUNEgmxa51799116 = -735229529;    double NlsdldfwnTUrrUNEgmxa96695425 = -562609254;    double NlsdldfwnTUrrUNEgmxa78891392 = -449181311;    double NlsdldfwnTUrrUNEgmxa63131071 = -377202853;    double NlsdldfwnTUrrUNEgmxa18497896 = -599234612;    double NlsdldfwnTUrrUNEgmxa96974128 = -954623395;    double NlsdldfwnTUrrUNEgmxa10670907 = -748858892;    double NlsdldfwnTUrrUNEgmxa63523498 = 43356535;    double NlsdldfwnTUrrUNEgmxa8542618 = -227095625;    double NlsdldfwnTUrrUNEgmxa53845444 = 37030410;    double NlsdldfwnTUrrUNEgmxa76378756 = -17437557;    double NlsdldfwnTUrrUNEgmxa79851628 = -18936103;    double NlsdldfwnTUrrUNEgmxa65014010 = -404559907;    double NlsdldfwnTUrrUNEgmxa13665017 = -570805026;    double NlsdldfwnTUrrUNEgmxa72486492 = -266258597;    double NlsdldfwnTUrrUNEgmxa65405629 = -838634330;    double NlsdldfwnTUrrUNEgmxa9638585 = -384324032;    double NlsdldfwnTUrrUNEgmxa62287783 = -85992256;    double NlsdldfwnTUrrUNEgmxa54495185 = -215460858;    double NlsdldfwnTUrrUNEgmxa74197725 = -409622727;    double NlsdldfwnTUrrUNEgmxa42294286 = -938470384;    double NlsdldfwnTUrrUNEgmxa81172205 = -183292162;    double NlsdldfwnTUrrUNEgmxa59493428 = -670147964;    double NlsdldfwnTUrrUNEgmxa96540690 = -583792087;    double NlsdldfwnTUrrUNEgmxa84397140 = -908193470;    double NlsdldfwnTUrrUNEgmxa74334713 = -514431048;    double NlsdldfwnTUrrUNEgmxa11408272 = -916767215;    double NlsdldfwnTUrrUNEgmxa16566779 = -985909324;    double NlsdldfwnTUrrUNEgmxa90231941 = -775519261;    double NlsdldfwnTUrrUNEgmxa80290829 = -943566020;    double NlsdldfwnTUrrUNEgmxa16354290 = -956028541;    double NlsdldfwnTUrrUNEgmxa80152041 = -893457427;    double NlsdldfwnTUrrUNEgmxa59240737 = -185708854;    double NlsdldfwnTUrrUNEgmxa7901510 = -382618479;    double NlsdldfwnTUrrUNEgmxa23227423 = -827892202;    double NlsdldfwnTUrrUNEgmxa50671221 = -585337144;    double NlsdldfwnTUrrUNEgmxa1806850 = -5567558;     NlsdldfwnTUrrUNEgmxa42868592 = NlsdldfwnTUrrUNEgmxa70772825;     NlsdldfwnTUrrUNEgmxa70772825 = NlsdldfwnTUrrUNEgmxa11541498;     NlsdldfwnTUrrUNEgmxa11541498 = NlsdldfwnTUrrUNEgmxa96171969;     NlsdldfwnTUrrUNEgmxa96171969 = NlsdldfwnTUrrUNEgmxa28192257;     NlsdldfwnTUrrUNEgmxa28192257 = NlsdldfwnTUrrUNEgmxa55310277;     NlsdldfwnTUrrUNEgmxa55310277 = NlsdldfwnTUrrUNEgmxa3528116;     NlsdldfwnTUrrUNEgmxa3528116 = NlsdldfwnTUrrUNEgmxa84681848;     NlsdldfwnTUrrUNEgmxa84681848 = NlsdldfwnTUrrUNEgmxa41383091;     NlsdldfwnTUrrUNEgmxa41383091 = NlsdldfwnTUrrUNEgmxa21513355;     NlsdldfwnTUrrUNEgmxa21513355 = NlsdldfwnTUrrUNEgmxa20131745;     NlsdldfwnTUrrUNEgmxa20131745 = NlsdldfwnTUrrUNEgmxa72060772;     NlsdldfwnTUrrUNEgmxa72060772 = NlsdldfwnTUrrUNEgmxa54721915;     NlsdldfwnTUrrUNEgmxa54721915 = NlsdldfwnTUrrUNEgmxa62559437;     NlsdldfwnTUrrUNEgmxa62559437 = NlsdldfwnTUrrUNEgmxa78157371;     NlsdldfwnTUrrUNEgmxa78157371 = NlsdldfwnTUrrUNEgmxa19193067;     NlsdldfwnTUrrUNEgmxa19193067 = NlsdldfwnTUrrUNEgmxa21832811;     NlsdldfwnTUrrUNEgmxa21832811 = NlsdldfwnTUrrUNEgmxa65021312;     NlsdldfwnTUrrUNEgmxa65021312 = NlsdldfwnTUrrUNEgmxa544911;     NlsdldfwnTUrrUNEgmxa544911 = NlsdldfwnTUrrUNEgmxa26166105;     NlsdldfwnTUrrUNEgmxa26166105 = NlsdldfwnTUrrUNEgmxa95985364;     NlsdldfwnTUrrUNEgmxa95985364 = NlsdldfwnTUrrUNEgmxa88691555;     NlsdldfwnTUrrUNEgmxa88691555 = NlsdldfwnTUrrUNEgmxa19853139;     NlsdldfwnTUrrUNEgmxa19853139 = NlsdldfwnTUrrUNEgmxa46103871;     NlsdldfwnTUrrUNEgmxa46103871 = NlsdldfwnTUrrUNEgmxa63172605;     NlsdldfwnTUrrUNEgmxa63172605 = NlsdldfwnTUrrUNEgmxa33370677;     NlsdldfwnTUrrUNEgmxa33370677 = NlsdldfwnTUrrUNEgmxa11296165;     NlsdldfwnTUrrUNEgmxa11296165 = NlsdldfwnTUrrUNEgmxa16454377;     NlsdldfwnTUrrUNEgmxa16454377 = NlsdldfwnTUrrUNEgmxa61612003;     NlsdldfwnTUrrUNEgmxa61612003 = NlsdldfwnTUrrUNEgmxa89583974;     NlsdldfwnTUrrUNEgmxa89583974 = NlsdldfwnTUrrUNEgmxa24817930;     NlsdldfwnTUrrUNEgmxa24817930 = NlsdldfwnTUrrUNEgmxa41240353;     NlsdldfwnTUrrUNEgmxa41240353 = NlsdldfwnTUrrUNEgmxa8929702;     NlsdldfwnTUrrUNEgmxa8929702 = NlsdldfwnTUrrUNEgmxa36224019;     NlsdldfwnTUrrUNEgmxa36224019 = NlsdldfwnTUrrUNEgmxa65585308;     NlsdldfwnTUrrUNEgmxa65585308 = NlsdldfwnTUrrUNEgmxa67486465;     NlsdldfwnTUrrUNEgmxa67486465 = NlsdldfwnTUrrUNEgmxa55669568;     NlsdldfwnTUrrUNEgmxa55669568 = NlsdldfwnTUrrUNEgmxa13290194;     NlsdldfwnTUrrUNEgmxa13290194 = NlsdldfwnTUrrUNEgmxa11175869;     NlsdldfwnTUrrUNEgmxa11175869 = NlsdldfwnTUrrUNEgmxa24166154;     NlsdldfwnTUrrUNEgmxa24166154 = NlsdldfwnTUrrUNEgmxa46314476;     NlsdldfwnTUrrUNEgmxa46314476 = NlsdldfwnTUrrUNEgmxa30971355;     NlsdldfwnTUrrUNEgmxa30971355 = NlsdldfwnTUrrUNEgmxa75026538;     NlsdldfwnTUrrUNEgmxa75026538 = NlsdldfwnTUrrUNEgmxa47366647;     NlsdldfwnTUrrUNEgmxa47366647 = NlsdldfwnTUrrUNEgmxa80698242;     NlsdldfwnTUrrUNEgmxa80698242 = NlsdldfwnTUrrUNEgmxa53534020;     NlsdldfwnTUrrUNEgmxa53534020 = NlsdldfwnTUrrUNEgmxa71082893;     NlsdldfwnTUrrUNEgmxa71082893 = NlsdldfwnTUrrUNEgmxa56800980;     NlsdldfwnTUrrUNEgmxa56800980 = NlsdldfwnTUrrUNEgmxa42256652;     NlsdldfwnTUrrUNEgmxa42256652 = NlsdldfwnTUrrUNEgmxa19317718;     NlsdldfwnTUrrUNEgmxa19317718 = NlsdldfwnTUrrUNEgmxa8411770;     NlsdldfwnTUrrUNEgmxa8411770 = NlsdldfwnTUrrUNEgmxa65324501;     NlsdldfwnTUrrUNEgmxa65324501 = NlsdldfwnTUrrUNEgmxa44699662;     NlsdldfwnTUrrUNEgmxa44699662 = NlsdldfwnTUrrUNEgmxa24532562;     NlsdldfwnTUrrUNEgmxa24532562 = NlsdldfwnTUrrUNEgmxa61889305;     NlsdldfwnTUrrUNEgmxa61889305 = NlsdldfwnTUrrUNEgmxa54177037;     NlsdldfwnTUrrUNEgmxa54177037 = NlsdldfwnTUrrUNEgmxa50919686;     NlsdldfwnTUrrUNEgmxa50919686 = NlsdldfwnTUrrUNEgmxa65437626;     NlsdldfwnTUrrUNEgmxa65437626 = NlsdldfwnTUrrUNEgmxa32999365;     NlsdldfwnTUrrUNEgmxa32999365 = NlsdldfwnTUrrUNEgmxa94821579;     NlsdldfwnTUrrUNEgmxa94821579 = NlsdldfwnTUrrUNEgmxa44014113;     NlsdldfwnTUrrUNEgmxa44014113 = NlsdldfwnTUrrUNEgmxa87073739;     NlsdldfwnTUrrUNEgmxa87073739 = NlsdldfwnTUrrUNEgmxa23069845;     NlsdldfwnTUrrUNEgmxa23069845 = NlsdldfwnTUrrUNEgmxa51799116;     NlsdldfwnTUrrUNEgmxa51799116 = NlsdldfwnTUrrUNEgmxa96695425;     NlsdldfwnTUrrUNEgmxa96695425 = NlsdldfwnTUrrUNEgmxa78891392;     NlsdldfwnTUrrUNEgmxa78891392 = NlsdldfwnTUrrUNEgmxa63131071;     NlsdldfwnTUrrUNEgmxa63131071 = NlsdldfwnTUrrUNEgmxa18497896;     NlsdldfwnTUrrUNEgmxa18497896 = NlsdldfwnTUrrUNEgmxa96974128;     NlsdldfwnTUrrUNEgmxa96974128 = NlsdldfwnTUrrUNEgmxa10670907;     NlsdldfwnTUrrUNEgmxa10670907 = NlsdldfwnTUrrUNEgmxa63523498;     NlsdldfwnTUrrUNEgmxa63523498 = NlsdldfwnTUrrUNEgmxa8542618;     NlsdldfwnTUrrUNEgmxa8542618 = NlsdldfwnTUrrUNEgmxa53845444;     NlsdldfwnTUrrUNEgmxa53845444 = NlsdldfwnTUrrUNEgmxa76378756;     NlsdldfwnTUrrUNEgmxa76378756 = NlsdldfwnTUrrUNEgmxa79851628;     NlsdldfwnTUrrUNEgmxa79851628 = NlsdldfwnTUrrUNEgmxa65014010;     NlsdldfwnTUrrUNEgmxa65014010 = NlsdldfwnTUrrUNEgmxa13665017;     NlsdldfwnTUrrUNEgmxa13665017 = NlsdldfwnTUrrUNEgmxa72486492;     NlsdldfwnTUrrUNEgmxa72486492 = NlsdldfwnTUrrUNEgmxa65405629;     NlsdldfwnTUrrUNEgmxa65405629 = NlsdldfwnTUrrUNEgmxa9638585;     NlsdldfwnTUrrUNEgmxa9638585 = NlsdldfwnTUrrUNEgmxa62287783;     NlsdldfwnTUrrUNEgmxa62287783 = NlsdldfwnTUrrUNEgmxa54495185;     NlsdldfwnTUrrUNEgmxa54495185 = NlsdldfwnTUrrUNEgmxa74197725;     NlsdldfwnTUrrUNEgmxa74197725 = NlsdldfwnTUrrUNEgmxa42294286;     NlsdldfwnTUrrUNEgmxa42294286 = NlsdldfwnTUrrUNEgmxa81172205;     NlsdldfwnTUrrUNEgmxa81172205 = NlsdldfwnTUrrUNEgmxa59493428;     NlsdldfwnTUrrUNEgmxa59493428 = NlsdldfwnTUrrUNEgmxa96540690;     NlsdldfwnTUrrUNEgmxa96540690 = NlsdldfwnTUrrUNEgmxa84397140;     NlsdldfwnTUrrUNEgmxa84397140 = NlsdldfwnTUrrUNEgmxa74334713;     NlsdldfwnTUrrUNEgmxa74334713 = NlsdldfwnTUrrUNEgmxa11408272;     NlsdldfwnTUrrUNEgmxa11408272 = NlsdldfwnTUrrUNEgmxa16566779;     NlsdldfwnTUrrUNEgmxa16566779 = NlsdldfwnTUrrUNEgmxa90231941;     NlsdldfwnTUrrUNEgmxa90231941 = NlsdldfwnTUrrUNEgmxa80290829;     NlsdldfwnTUrrUNEgmxa80290829 = NlsdldfwnTUrrUNEgmxa16354290;     NlsdldfwnTUrrUNEgmxa16354290 = NlsdldfwnTUrrUNEgmxa80152041;     NlsdldfwnTUrrUNEgmxa80152041 = NlsdldfwnTUrrUNEgmxa59240737;     NlsdldfwnTUrrUNEgmxa59240737 = NlsdldfwnTUrrUNEgmxa7901510;     NlsdldfwnTUrrUNEgmxa7901510 = NlsdldfwnTUrrUNEgmxa23227423;     NlsdldfwnTUrrUNEgmxa23227423 = NlsdldfwnTUrrUNEgmxa50671221;     NlsdldfwnTUrrUNEgmxa50671221 = NlsdldfwnTUrrUNEgmxa1806850;     NlsdldfwnTUrrUNEgmxa1806850 = NlsdldfwnTUrrUNEgmxa42868592;}
// Junk Finished
