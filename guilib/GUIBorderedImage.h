#ifndef CGUIBorderedImage_H
#define CGUIBorderedImage_H

#include "GUIControl.h"
#include "TextureManager.h"
#include "guiImage.h"

class CGUIBorderedImage : public CGUIImage
{
public:
  CGUIBorderedImage(DWORD dwParentID, DWORD dwControlId, float posX, float posY, float width, float height, const CImage& texture, const CImage& borderTexture, float borderSize, DWORD dwColorKey = 0);
  virtual ~CGUIBorderedImage(void);

  virtual void Render();
  virtual void UpdateVisibility();
  virtual bool OnMessage(CGUIMessage& message);
  virtual void PreAllocResources();
  virtual void AllocResources();
  virtual void FreeResources();
  virtual void DynamicResourceAlloc(bool bOnOff);
  virtual bool IsAllocated() const;
  
protected:
  CGUIImage m_borderImage;
  float m_borderSize;
};

#endif
