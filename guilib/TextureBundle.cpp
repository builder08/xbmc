

#include "include.h"
#include "./TextureBundle.h"
#include "GraphicContext.h"
#include "DirectXGraphics.h"
#ifndef _LINUX
#include <sys/stat.h>
#include "utils/CharsetConverter.h"
#include "lib/liblzo/LZO1X.H"
#else
#include <lzo/lzo1x.h>
#endif
#include "SkinInfo.h"
#include "GUISettings.h"
#include "Util.h"
#include "FileSystem/SpecialProtocol.h"

#if !defined(__GNUC__)
#pragma comment(lib,"../../xbmc/lib/liblzo/lzo.lib")
#endif

// alignment of file blocks - should be a multiple of the sector size of the disk and a power of 2
// HDD sector = 512 bytes, DVD/CD sector = 2048 bytes
#undef ALIGN
#define ALIGN (512)

enum XPR_FLAGS
{
  XPRFLAG_PALETTE = 0x00000001,
  XPRFLAG_ANIM = 0x00000002
};

class CAutoBuffer
{
  BYTE* p;
public:
  CAutoBuffer() { p = 0; }
  explicit CAutoBuffer(size_t s) { p = (BYTE*)malloc(s); }
  ~CAutoBuffer() { free(p); }
operator BYTE*() { return p; }
  void Set(BYTE* buf) { free(p); p = buf; }
  bool Resize(size_t s);
void Release() { p = 0; }
};

bool CAutoBuffer::Resize(size_t s)
{
  if (s == 0)
  {
    if (!p)
      return false;
    free(p);
    p = 0;
    return true;
  }
  void* q = realloc(p, s);
  if (q)
  {
    p = (BYTE*)q;
    return true;
  }
  return false;
}

// as above but for texture allocation (do not change from XPhysicalAlloc!)
class CAutoTexBuffer
{
  BYTE* p;
public:
  CAutoTexBuffer() { p = 0; }
  explicit CAutoTexBuffer(size_t s) { p = (BYTE*)XPhysicalAlloc(s, MAXULONG_PTR, 128, PAGE_READWRITE); }
  ~CAutoTexBuffer() { if (p) XPhysicalFree(p); }
operator BYTE*() { return p; }
  BYTE* Set(BYTE* buf) { if (p) XPhysicalFree(p); return p = buf; }
void Release() { p = 0; }
};

CTextureBundle::CTextureBundle(void)
{
  m_hFile = NULL;
  m_themeBundle = false;
}

CTextureBundle::~CTextureBundle(void)
{
  if (m_hFile != NULL)
    fclose(m_hFile);
}

bool CTextureBundle::OpenBundle()
{
  DWORD AlignedSize;
  DWORD HeaderSize;
  int Version;
  XPR_HEADER* pXPRHeader;

  if (m_hFile != NULL)
    Cleanup();

  CStdString strPath;

  if (m_themeBundle)
  {
    // if we are the theme bundle, we only load if the user has chosen
    // a valid theme (or the skin has a default one)
    CStdString themeXPR = g_guiSettings.GetString("lookandfeel.skintheme");
    if (!themeXPR.IsEmpty() && themeXPR.CompareNoCase("SKINDEFAULT"))
    {
      strPath = CUtil::AddFileToFolder(g_graphicsContext.GetMediaDir(), "media");
      strPath = CUtil::AddFileToFolder(strPath, themeXPR);
    }
    else
      return false;
  }
  else
    strPath = CUtil::AddFileToFolder(g_graphicsContext.GetMediaDir(), "media/Textures.xpr");

  strPath = PTH_IC(strPath);

#ifndef _LINUX
  CStdStringW strPathW;
  g_charsetConverter.utf8ToW(_P(strPath), strPathW, false);
  m_hFile = _wfopen(strPathW.c_str(), L"rb");
#else
  m_hFile = fopen(strPath.c_str(), "rb");
#endif
  if (m_hFile == NULL)
    return false;

  struct stat fileStat;
  if (fstat(fileno(m_hFile), &fileStat) == -1)
    return false;
  m_TimeStamp = fileStat.st_mtime;

  CAutoBuffer HeaderBuf(ALIGN);
  DWORD n;

  n = fread(HeaderBuf, 1, ALIGN, m_hFile);
  if (n < ALIGN)
    goto LoadError;

  pXPRHeader = (XPR_HEADER*)(BYTE*)HeaderBuf;
  Version = (pXPRHeader->dwMagic >> 24) - '0';
  pXPRHeader->dwMagic -= Version << 24;
  Version &= 0x0f;

  if (pXPRHeader->dwMagic != XPR_MAGIC_VALUE || Version < 2)
    goto LoadError;

  HeaderSize = pXPRHeader->dwHeaderSize;
  AlignedSize = (HeaderSize - 1) & ~(ALIGN - 1); // align to sector, but remove the first sector
  HeaderBuf.Resize(AlignedSize + ALIGN);

  if (fseek(m_hFile, ALIGN, SEEK_SET) == -1)
    goto LoadError;
  n = fread(HeaderBuf + ALIGN, 1, AlignedSize, m_hFile);
  if (n < ALIGN)
    goto LoadError;

  struct DiskFileHeader_t
  {
    char Name[116];
    DWORD Offset;
    DWORD UnpackedSize;
    DWORD PackedSize;
  }
  *FileHeader;
  FileHeader = (DiskFileHeader_t*)(HeaderBuf + sizeof(XPR_HEADER));

  n = (HeaderSize - sizeof(XPR_HEADER)) / sizeof(DiskFileHeader_t);
  for (unsigned i = 0; i < n; ++i)
  {
    std::pair<CStdString, FileHeader_t> entry;
    entry.first = Normalize(FileHeader[i].Name);
    entry.second.Offset = FileHeader[i].Offset;
    entry.second.UnpackedSize = FileHeader[i].UnpackedSize;
    entry.second.PackedSize = FileHeader[i].PackedSize;
    m_FileHeaders.insert(entry);
  }

  if (lzo_init() != LZO_E_OK)
    goto LoadError;

  return true;

LoadError:
  CLog::Log(LOGERROR, "Unable to load file: %s: %s", strPath.c_str(), strerror(errno));
  fclose(m_hFile);
  m_hFile = NULL;

  return false;
}

void CTextureBundle::Cleanup()
{
  if (m_hFile != NULL)
    fclose(m_hFile);
  m_hFile = NULL;

  m_FileHeaders.clear();
}

bool CTextureBundle::HasFile(const CStdString& Filename)
{
  if (m_hFile == NULL && !OpenBundle())
    return false;

  struct stat fileStat;
  if (fstat(fileno(m_hFile), &fileStat) == -1)
    return false;
  if (fileStat.st_mtime > m_TimeStamp)
  {
    CLog::Log(LOGINFO, "Texture bundle has changed, reloading");
    Cleanup();
    if (!OpenBundle())
      return false;
  }

  CStdString name = Normalize(Filename);
  return m_FileHeaders.find(name) != m_FileHeaders.end();
}

void CTextureBundle::GetTexturesFromPath(const CStdString &path, std::vector<CStdString> &textures)
{
  if (path.GetLength() > 1 && path[1] == ':')
    return;

  if (m_hFile == NULL && !OpenBundle())
    return;

  CStdString testPath = Normalize(path);
  if (!CUtil::HasSlashAtEnd(testPath))
    testPath += "\\";
  int testLength = testPath.GetLength();
  std::map<CStdString, FileHeader_t>::iterator it;
  for (it = m_FileHeaders.begin(); it != m_FileHeaders.end(); it++)
  {
    if (it->first.Left(testLength).Equals(testPath))
      textures.push_back(it->first);
  }
}

HRESULT CTextureBundle::LoadFile(const CStdString& Filename, CAutoTexBuffer& UnpackedBuf)
{
  if (Filename == "-")
    return 0;

  CStdString name = Normalize(Filename);

  std::map<CStdString, FileHeader_t>::iterator file = m_FileHeaders.find(name);
  if (file == m_FileHeaders.end())
    return E_FAIL;

  // found texture - allocate the necessary buffers
  DWORD ReadSize = (file->second.PackedSize + (ALIGN - 1)) & ~(ALIGN - 1);
  BYTE *buffer = (BYTE*)malloc(ReadSize);
  
  if (!buffer || !UnpackedBuf.Set((BYTE*)XPhysicalAlloc(file->second.UnpackedSize, MAXULONG_PTR, 128, PAGE_READWRITE)))
  { // failed due to lack of memory
#ifndef _LINUX
    MEMORYSTATUS stat;
    GlobalMemoryStatus(&stat);
    CLog::Log(LOGERROR, "Out of memory loading texture: %s (need %lu bytes, have %lu bytes)", name.c_str(),
              file->second.UnpackedSize + file->second.PackedSize, stat.dwAvailPhys);
#elif defined(__APPLE__)
    CLog::Log(LOGERROR, "Out of memory loading texture: %s (need %lu bytes)", name.c_str(),
              file->second.UnpackedSize + file->second.PackedSize);
#else
    struct sysinfo info;
    sysinfo(&info);
    CLog::Log(LOGERROR, "Out of memory loading texture: %s "
                        "(need %u bytes, have %lu bytes)",
              name.c_str(), file->second.UnpackedSize + file->second.PackedSize,
              info.totalram);
#endif
    free(buffer);
    return E_OUTOFMEMORY;
  }

  // read the file into our buffer
  DWORD n;
  fseek(m_hFile, file->second.Offset, SEEK_SET);
  n = fread(buffer, 1, ReadSize, m_hFile);
  if (n < ReadSize && !feof(m_hFile))
  {
    CLog::Log(LOGERROR, "Error loading texture: %s: %s", Filename.c_str(), strerror(ferror(m_hFile)));
    free(buffer);
    return E_FAIL;
  }

  // allocate a buffer for our unpacked texture
  lzo_uint s = file->second.UnpackedSize;
  HRESULT hr = S_OK;
  if (lzo1x_decompress(buffer, file->second.PackedSize, UnpackedBuf, &s, NULL) != LZO_E_OK ||
      s != file->second.UnpackedSize)
  {
    CLog::Log(LOGERROR, "Error loading texture: %s: Decompression error", Filename.c_str());
    hr = E_FAIL;
  }

  try
  {
    free(buffer);
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "Error freeing preload buffer.");
  }

  return hr;
}

HRESULT CTextureBundle::LoadTexture(const CStdString& Filename, CBaseTexture** ppTexture,
                                     XBMC::PalettePtr* ppPalette)
{
  DWORD ResDataOffset;
  *ppTexture = NULL; *ppPalette = NULL;

  CAutoTexBuffer UnpackedBuf;
  HRESULT r = LoadFile(Filename, UnpackedBuf);
  if (r != S_OK)
    return r;

  D3DTexture *pTex = (D3DTexture *)(new char[sizeof (D3DTexture)]);
  D3DPalette* pPal = 0;
  void* ResData = 0;

  WORD RealSize[2];

  enum XPR_FLAGS
  {
    XPRFLAG_PALETTE = 0x00000001,
    XPRFLAG_ANIM = 0x00000002
  };

  BYTE* Next = UnpackedBuf;

  DWORD flags = *(DWORD*)Next;
  Next += sizeof(DWORD);
  if ((flags & XPRFLAG_ANIM) || (flags >> 16) > 1)
    goto PackedLoadError;

  if (flags & XPRFLAG_PALETTE)
  {
    pPal = new D3DPalette;
    memcpy(pPal, Next, sizeof(D3DPalette));
    Next += sizeof(D3DPalette);
  }

  memcpy(pTex, Next, sizeof(D3DTexture));
  Next += sizeof(D3DTexture);

  memcpy(RealSize, Next, 4);
  Next += 4;

  ResDataOffset = ((Next - UnpackedBuf) + 127) & ~127;
  ResData = UnpackedBuf + ResDataOffset;

  if ((pTex->Common & D3DCOMMON_TYPE_MASK) != D3DCOMMON_TYPE_TEXTURE)
    goto PackedLoadError;

  GetTextureFromData(pTex, ResData, ppTexture);
  delete[] pTex;
  delete pPal;

/* DXMERGE - this was previously used to specify the real size
             of the image - is it actually being used still??
  pInfo->Width = RealSize[0];
  pInfo->Height = RealSize[1];
  pInfo->Depth = 0;
  pInfo->MipLevels = 1;
#ifndef HAS_SDL
  D3DSURFACE_DESC desc;
  (*ppTexture)->GetLevelDesc(0, &desc);
  pInfo->Format = desc.Format;
#endif
*/

  return S_OK;

PackedLoadError:
  CLog::Log(LOGERROR, "Error loading texture: %s: Invalid data", Filename.c_str());
  delete[] pTex;
  delete pPal;
  return E_FAIL;
}

int CTextureBundle::LoadAnim(const CStdString& Filename, CBaseTexture** ppTextures,
                              XBMC::PalettePtr* ppPalette, int& nLoops, int** ppDelays)
{
  DWORD ResDataOffset;
  int nTextures = 0;

  *ppTextures = NULL; *ppPalette = NULL; *ppDelays = NULL;

  CAutoTexBuffer UnpackedBuf;
  HRESULT r = LoadFile(Filename, UnpackedBuf);
  if (r != S_OK)
    return 0;

  struct AnimInfo_t
  {
    DWORD nLoops;
    WORD RealSize[2];
  }
  *pAnimInfo;

  D3DTexture** ppTex = 0;
  D3DPalette* pPal = 0;
  void* ResData = 0;

  BYTE* Next = UnpackedBuf;

  DWORD flags = *(DWORD*)Next;
  Next += sizeof(DWORD);
  if (!(flags & XPRFLAG_ANIM))
    goto PackedAnimError;

  pAnimInfo = (AnimInfo_t*)Next;
  Next += sizeof(AnimInfo_t);
  nLoops = pAnimInfo->nLoops;

  if (flags & XPRFLAG_PALETTE)
  {
    pPal = new D3DPalette;
    memcpy(pPal, Next, sizeof(D3DPalette));
    Next += sizeof(D3DPalette);
  }

  nTextures = flags >> 16;
  ppTex = new D3DTexture * [nTextures];
  *ppDelays = new int[nTextures];
  for (int i = 0; i < nTextures; ++i)
  {
    ppTex[i] = (D3DTexture *)(new char[sizeof (D3DTexture)+ sizeof (DWORD)]);

    memcpy(ppTex[i], Next, sizeof(D3DTexture));
    Next += sizeof(D3DTexture);

    (*ppDelays)[i] = *(int*)Next;
    Next += sizeof(int);
  }

  ResDataOffset = ((DWORD)(Next - UnpackedBuf) + 127) & ~127;
  ResData = UnpackedBuf + ResDataOffset;

  *ppTextures = new CTexture[nTextures];
  for (int i = 0; i < nTextures; ++i)
  {
    if ((ppTex[i]->Common & D3DCOMMON_TYPE_MASK) != D3DCOMMON_TYPE_TEXTURE)
      goto PackedAnimError;

    GetTextureFromData(ppTex[i], ResData, &(ppTextures)[i]);
    delete[] ppTex[i];
  }

  delete[] ppTex;
  ppTex = 0;
  delete pPal;
/* DXMERGE - this was previously used to specify the real size
             of the image - is it actually being used still??
  pInfo->Width = pAnimInfo->RealSize[0];
  pInfo->Height = pAnimInfo->RealSize[1];
  pInfo->Depth = 0;
  pInfo->MipLevels = 1;
  pInfo->Format = D3DFMT_UNKNOWN;
  */

  return nTextures;

PackedAnimError:
  CLog::Log(LOGERROR, "Error loading texture: %s: Invalid data", Filename.c_str());
  if (ppTex)
  {
    for (int i = 0; i < nTextures; ++i)
      delete [] ppTex[i];
    delete [] ppTex;
  }
  delete pPal;
  delete [] *ppDelays;
  return 0;
}

void CTextureBundle::SetThemeBundle(bool themeBundle)
{
  m_themeBundle = themeBundle;
}

// normalize to how it's stored within the bundle
// lower case + using \\ rather than /
CStdString CTextureBundle::Normalize(const CStdString &name)
{
  CStdString newName(name);
  newName.Normalize();
  newName.Replace('/','\\');
  return newName;
}
