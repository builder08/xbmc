/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "utils/ColorUtils.h"
#include "utils/Geometry.h"

#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <harfbuzz/hb.h>

#ifdef HAS_DX
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
#endif

class CTexture;
class CRenderSystemBase;

struct FT_FaceRec_;
struct FT_LibraryRec_;
struct FT_GlyphSlotRec_;
struct FT_BitmapGlyphRec_;
struct FT_StrokerRec_;

typedef struct FT_FaceRec_* FT_Face;
typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_GlyphSlotRec_* FT_GlyphSlot;
typedef struct FT_BitmapGlyphRec_* FT_BitmapGlyph;
typedef struct FT_StrokerRec_* FT_Stroker;

typedef uint32_t character_t;
typedef std::vector<character_t> vecText;

/*!
 \ingroup textures
 \brief
 */

struct SVertex
{
  float x, y, z;
#ifdef HAS_DX
  XMFLOAT4 col;
#else
  unsigned char r, g, b, a;
#endif
  float u, v;
};


#include "GUIFontCache.h"


class CGUIFontTTF
{
  static constexpr size_t LOOKUPTABLE_SIZE = 256 * 8;

  friend class CGUIFont;

public:
  virtual ~CGUIFontTTF();

  static CGUIFontTTF* CreateGUIFontTTF(const std::string& fontIdent);

  void Clear();

  bool Load(const std::string& strFilename,
            float height = 20.0f,
            float aspect = 1.0f,
            float lineSpacing = 1.0f,
            bool border = false);

  void Begin();
  void End();
  /* The next two should only be called if we've declared we can do hardware clipping */
  virtual CVertexBuffer CreateVertexBuffer(const std::vector<SVertex>& vertices) const
  {
    assert(false);
    return CVertexBuffer();
  }
  virtual void DestroyVertexBuffer(CVertexBuffer& bufferHandle) const {}

  const std::string& GetFontIdent() const { return m_fontIdent; }

protected:
  explicit CGUIFontTTF(const std::string& fontIdent);


  struct Glyph
  {
    hb_glyph_info_t glyphInfo;
    hb_glyph_position_t glyphPosition;

    // converter for harfbuzz library
    Glyph(hb_glyph_info_t gInfo, hb_glyph_position_t gPos)
    {
      glyphInfo = gInfo;
      glyphPosition = gPos;
    }
    Glyph() {}
  };

  struct Character
  {
    short offsetX, offsetY;
    float left, top, right, bottom;
    float advance;
    FT_UInt glyphIndex;
    character_t glyphAndStyle;
    wchar_t letter;
  };

  struct RunInfo
  {
    unsigned int startOffset;
    unsigned int endOffset;
    hb_buffer_t* buffer;
    hb_script_t script;
    hb_glyph_info_t* glyphInfos;
    hb_glyph_position_t* glyphPositions;
  };

  void AddReference();
  void RemoveReference();

  std::vector<Glyph> GetHarfBuzzShapedGlyphs(const vecText& text);

  float GetTextWidthInternal(const vecText& text);
  float GetTextWidthInternal(const vecText& text, std::vector<Glyph>& glyph);
  float GetCharWidthInternal(character_t ch);
  float GetTextHeight(float lineSpacing, int numLines) const;
  float GetTextBaseLine() const { return static_cast<float>(m_cellBaseLine); }
  float GetLineHeight(float lineSpacing) const;
  float GetFontHeight() const { return m_height; }

  void DrawTextInternal(float x,
                        float y,
                        const std::vector<UTILS::COLOR::Color>& colors,
                        const vecText& text,
                        uint32_t alignment,
                        float maxPixelWidth,
                        bool scrolling);

  float m_height{0.0f};

  // Stuff for pre-rendering for speed
  Character* GetCharacter(character_t letter, FT_UInt glyphIndex);
  bool CacheCharacter(wchar_t letter, uint32_t style, Character* ch, FT_UInt glyphIndex);
  void RenderCharacter(float posX,
                       float posY,
                       const Character* ch,
                       UTILS::COLOR::Color color,
                       bool roundX,
                       std::vector<SVertex>& vertices);
  void ClearCharacterCache();

  virtual std::unique_ptr<CTexture> ReallocTexture(unsigned int& newHeight) = 0;
  virtual bool CopyCharToTexture(FT_BitmapGlyph bitGlyph,
                                 unsigned int x1,
                                 unsigned int y1,
                                 unsigned int x2,
                                 unsigned int y2) = 0;
  virtual void DeleteHardwareTexture() = 0;

  // modifying glyphs
  void SetGlyphStrength(FT_GlyphSlot slot, int glyphStrength);
  static void ObliqueGlyph(FT_GlyphSlot slot);

  std::unique_ptr<CTexture>
      m_texture; // texture that holds our rendered characters (8bit alpha only)

  unsigned int m_textureWidth{0}; // width of our texture
  unsigned int m_textureHeight{0}; // height of our texture
  int m_posX{0}; // current position in the texture
  int m_posY{0};

  /*! \brief the height of each line in the texture.
   Accounts for spacing between lines to avoid characters overlapping.
   */
  unsigned int GetTextureLineHeight() const;

  UTILS::COLOR::Color m_color{UTILS::COLOR::NONE};

  Character* m_char{nullptr}; // our characters
  Character* m_charquick[LOOKUPTABLE_SIZE]{nullptr}; // ascii chars (7 styles) here
  int m_maxChars{0}; // size of character array (can be incremented)
  int m_numChars{0}; // the current number of cached characters

  float m_ellipsesWidth{0.0f}; // this is used every character (width of '.')

  unsigned int m_cellBaseLine{0};
  unsigned int m_cellHeight{0};

  unsigned int m_nestedBeginCount{0}; // speedups

  // freetype stuff
  FT_Face m_face{nullptr};
  FT_Stroker m_stroker{nullptr};

  hb_font_t* m_hbFont{nullptr};

  float m_originX{0.0f};
  float m_originY{0.0f};

  unsigned int m_nTexture{0};

  struct CTranslatedVertices
  {
    float translateX;
    float translateY;
    float translateZ;
    const CVertexBuffer* vertexBuffer;
    CRect clip;
    CTranslatedVertices(float translateX,
                        float translateY,
                        float translateZ,
                        const CVertexBuffer* vertexBuffer,
                        const CRect& clip)
      : translateX(translateX),
        translateY(translateY),
        translateZ(translateZ),
        vertexBuffer(vertexBuffer),
        clip(clip)
    {
    }
  };
  std::vector<CTranslatedVertices> m_vertexTrans;
  std::vector<SVertex> m_vertex;

  float m_textureScaleX{0.0f};
  float m_textureScaleY{0.0f};

  const std::string m_fontIdent;
  std::vector<uint8_t>
      m_fontFileInMemory; // used only in some cases, see CFreeTypeLibrary::GetFont()

  CGUIFontCache<CGUIFontCacheStaticPosition, CGUIFontCacheStaticValue> m_staticCache;
  CGUIFontCache<CGUIFontCacheDynamicPosition, CGUIFontCacheDynamicValue> m_dynamicCache;

  CRenderSystemBase* m_renderSystem;

private:
  float GetTabSpaceLength();

  virtual bool FirstBegin() = 0;
  virtual void LastEnd() = 0;
  CGUIFontTTF(const CGUIFontTTF&) = delete;
  CGUIFontTTF& operator=(const CGUIFontTTF&) = delete;
  int m_referenceCount{0};
};
