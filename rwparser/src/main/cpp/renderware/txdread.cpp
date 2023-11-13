#include <cstring>
#include <cstdlib>
#include <fstream>
#include "renderware.h"

using namespace std;

namespace rw {

    /*
     * Texture Dictionary
     */

    void TextureDictionary::read(istream &rw) {
        HeaderInfo header{};
        header.read(rw);
        if (header.type != CHUNK_TEXDICTIONARY) return;
        READ_HEADER(CHUNK_STRUCT)
        uint32 textureCount = readUInt16(rw);
        rw.seekg(2, ios::cur);
        texList.resize(textureCount);
        for (uint32 i = 0; i < textureCount; i++) {
            READ_HEADER(CHUNK_TEXTURENATIVE)
            rw.seekg(0x0c, ios::cur);
            texList[i].platform = readUInt32(rw);
            rw.seekg(-0x10, ios::cur);
            if (texList[i].platform == PLATFORM_XBOX) {
                texList[i].readXbox(rw);
            } else if (texList[i].platform == PLATFORM_D3D8 || texList[i].platform == PLATFORM_D3D9) {
                texList[i].readD3d(rw);
            } else if (texList[i].platform == PLATFORM_PS2FOURCC) {
                texList[i].platform = PLATFORM_PS2;
                texList[i].readPs2(rw);
            }
            READ_HEADER(CHUNK_EXTENSION)
            uint32 end = header.length;
            end += rw.tellg();
            while (rw.tellg() < end) {
                header.read(rw);
                switch (header.type) {
                    case CHUNK_SKYMIPMAP:
                        rw.seekg(4, ios::cur);
                        break;
                    default:
                        rw.seekg(header.length, ios::cur);
                        break;
                }
            }
        }
    }

    TextureDictionary::~TextureDictionary() { texList.clear(); }

    /*
     * Native Texture
     */

    void NativeTexture::readD3d(istream &rw) {
        HeaderInfo header{};
        READ_HEADER(CHUNK_STRUCT)
        uint32 platform = readUInt32(rw);
        if (platform != PLATFORM_D3D8 && platform != PLATFORM_D3D9) return;
        filterFlags = readUInt32(rw);
        char buffer[32];
        rw.read(buffer, 32);
        name = buffer;
        rw.read(buffer, 32);
        maskName = buffer;
        rasterFormat = readUInt32(rw);
        hasAlpha = false;
        char fourcc[5];
        fourcc[4] = 0;
        if (platform == PLATFORM_D3D9) {
            rw.read(fourcc, 4 * sizeof(char));
        } else {
            hasAlpha = readUInt32(rw);
        }
        width.push_back(readUInt16(rw));
        height.push_back(readUInt16(rw));
        depth = readUInt8(rw);
        mipmapCount = readUInt8(rw);
        rw.seekg(sizeof(int8), ios::cur);
        dxtCompression = readUInt8(rw);
        if (platform == PLATFORM_D3D9) {
            hasAlpha = dxtCompression & 0x1;
            if (dxtCompression & 0x8) dxtCompression = fourcc[3] - '0'; else dxtCompression = 0;
        }
        if (rasterFormat & RASTER_PAL8 || rasterFormat & RASTER_PAL4) {
            paletteSize = (rasterFormat & RASTER_PAL8) ? 0x100 : 0x10;
            palette = new uint8[paletteSize * 4 * sizeof(uint8)];
            rw.read(reinterpret_cast <char *> (palette),paletteSize * 4 * sizeof(uint8));
        }
        for (uint32 i = 0; i < mipmapCount; i++) {
            if (i > 0) {
                width.push_back(width[i - 1] / 2);
                height.push_back(height[i - 1] / 2);
                if (dxtCompression) {
                    if (width[i] < 4 && width[i] != 0) {
                        width[i] = 4;
                    }
                    if (height[i] < 4 && height[i] != 0) {
                        height[i] = 4;
                    }
                }
            }
            uint32 dataSize = readUInt32(rw);
            if (dataSize == 0) width[i] = height[i] = 0;
            dataSizes.push_back(dataSize);
            texels.push_back(new uint8[dataSize]);
            rw.read(reinterpret_cast <char *> (&texels[i][0]),dataSize * sizeof(uint8));
        }
    }

    void NativeTexture::readXbox(istream &rw) {
        HeaderInfo header{};
        READ_HEADER(CHUNK_STRUCT)
        uint32 platform = readUInt32(rw);
        if (platform != PLATFORM_XBOX) return;
        filterFlags = readUInt32(rw);
        char buffer[32];
        rw.read(buffer, 32);
        name = buffer;
        rw.read(buffer, 32);
        maskName = buffer;
        rasterFormat = readUInt32(rw);
        hasAlpha = readUInt32(rw);
        width.push_back(readUInt16(rw));
        height.push_back(readUInt16(rw));
        depth = readUInt8(rw);
        mipmapCount = readUInt8(rw);
        rw.seekg(sizeof(int8), ios::cur);
        dxtCompression = readUInt8(rw);
        rw.seekg(sizeof(int32), ios::cur);
        paletteSize = (rasterFormat & RASTER_PAL8) ? 0x100 : ((rasterFormat & RASTER_PAL4) ? 0x20 : 0);
        palette = new uint8[paletteSize * 4];
        rw.read(reinterpret_cast <char *> (&palette[0]),paletteSize * 4 * sizeof(uint8));
        for (uint32 i = 0; i < mipmapCount; i++) {
            if (i != 0) {
                width.push_back(width[i-1] / 2);
                height.push_back(height[i-1] / 2);
                if (dxtCompression) {
                    if (width[i] < 4) width[i] = 4;
                    if (height[i] < 4) height[i] = 4;
                }
            }
            dataSizes.push_back(width[i] * height[i]);
            if (dxtCompression == 0) {
                dataSizes[i] *= (depth / 8);
            } else if (dxtCompression == 0xC) {
                dataSizes[i] /= 2;
            }
            texels.push_back(new uint8[dataSizes[i]]);
            rw.read(reinterpret_cast <char *> (&texels[i][0]),dataSizes[i] * sizeof(uint8));
        }
    }

    void unswizzleXboxBlock(
        uint8 *out,
        uint8 *in,
        uint32 &outOff,
        uint32 inOff,
        uint32 width,
        uint32 height,
        uint32 stride
    ) {
        if (width < 2 || height < 2) {
            memcpy(out + outOff, in + inOff, width * height);
            inOff += width * height;
        } else if (width == 2 && height == 2) {
            *(out + outOff) = *(in + inOff);
            *(out + outOff + 1) = *(in + inOff + 1);
            *(out + outOff + stride) = *(in + inOff + 2);
            *(out + outOff + stride + 1) = *(in + inOff + 3);
            inOff += 4;
        } else {
            unswizzleXboxBlock(
                out,
                in,
                inOff,
                outOff,
                width / 2,
                height / 2,
                stride
            );
            unswizzleXboxBlock(
                out,
                in,
                inOff,
                outOff + (width / 2),
                width / 2,
                height / 2,
                stride
            );
            unswizzleXboxBlock(
                out,
                in,
                inOff,
                outOff + (height / 2) * stride,
                width / 2,
                height / 2,
                stride
            );
            unswizzleXboxBlock(
                out,
                in,
                inOff,
                outOff + (height / 2) * stride + (width / 2),
                width / 2,
                height / 2,
                stride
            );
        }
    }

    void NativeTexture::readPs2(istream &rw) {
        HeaderInfo header{};
        READ_HEADER(CHUNK_STRUCT)
        uint32 platform = readUInt32(rw);
        if (platform != PLATFORM_PS2FOURCC) return;
        paletteSize = 0;
        filterFlags = readUInt32(rw);
        READ_HEADER(CHUNK_STRING)
        char *buffer = new char[header.length + 1];
        rw.read(buffer, header.length);
        name = buffer;
        delete[] buffer;
        READ_HEADER(CHUNK_STRING)
        buffer = new char[header.length + 1];
        rw.read(buffer, header.length);
        maskName = buffer;
        delete[] buffer;
        READ_HEADER(CHUNK_STRUCT)
        READ_HEADER(CHUNK_STRUCT)
        width.push_back(readUInt32(rw));
        height.push_back(readUInt32(rw));
        depth = readUInt32(rw);
        rasterFormat = readUInt32(rw);
        rw.seekg(4 * 4, ios::cur);
        uint32 dataSize = readUInt32(rw);
        rw.seekg(4, ios::cur);
        bool hasHeader = (rasterFormat & 0x20000);
        hasAlpha = false;
        if (!maskName.empty()) hasAlpha = true;
        if (depth == 16) hasAlpha = true;
        READ_HEADER(CHUNK_STRUCT)
        uint32 end = rw.tellg();
        end += dataSize;
        uint32 i = 0;
        while (rw.tellg() < end) {
            if (i > 0) {
                width.push_back(width[i - 1] / 2);
                height.push_back(height[i - 1] / 2);
            }
            if (hasHeader) {
                rw.seekg(8 * 4, ios::cur);
                swizzleWidth.push_back(readUInt32(rw));
                swizzleHeight.push_back(readUInt32(rw));
                rw.seekg(6 * 4, ios::cur);
                dataSize = readUInt32(rw) * 0x10;
                rw.seekg(3 * 4, ios::cur);
            } else {
                swizzleWidth.push_back(width[i]);
                swizzleHeight.push_back(height[i]);
                if (rasterFormat & 0x10000) {
                    swizzleWidth[i] /= 2;
                    swizzleHeight[i] /= 2;
                }
                dataSize = height[i] * height[i] * depth / 8;
            }
            dataSizes.push_back(dataSize);
            texels.push_back(new uint8[dataSize]);
            rw.read(reinterpret_cast <char *> (&texels[i][0]),dataSize * sizeof(uint8));
            i++;
        }
        mipmapCount = i;
        if (rasterFormat & RASTER_PAL8 || rasterFormat & RASTER_PAL4) {
            uint32 unkh2 = 0, unkh3, unkh4;
            if (hasHeader) {
                rw.seekg(5 * 4, ios::cur);
                rw.seekg(2 * 4, ios::cur);
                unkh2 = readUInt32(rw);
                unkh3 = readUInt32(rw);
                rw.seekg(6 * 4, ios::cur);
                unkh4 = readUInt32(rw);
                rw.seekg(3 * 4, ios::cur);
            }
            paletteSize = (rasterFormat & RASTER_PAL8) ? 0x100 : 0x10;
            palette = new uint8[paletteSize * 4];
            rw.read(reinterpret_cast <char *> (palette),paletteSize * 4 * sizeof(uint8));
            if (unkh2 == 8 && unkh3 == 3 && unkh4 == 6) rw.seekg(0x20, ios::cur);
        }
        rasterFormat &= 0xff00;
        if ((rasterFormat & RASTER_8888) && !hasAlpha) {
            rasterFormat &= ~RASTER_8888;
            rasterFormat |= RASTER_888;
        }
    }

    NativeTexture::NativeTexture() : platform(0),
      name(""),
      maskName(""),
      filterFlags(0),
      rasterFormat(0),
      depth(0),
      palette(nullptr),
      paletteSize(0),
      hasAlpha(false),
      mipmapCount(0),
      alphaDistribution(0),
      dxtCompression(0)
    {}

    NativeTexture::NativeTexture(const NativeTexture &orig) : platform(orig.platform),
      name(orig.name),
      maskName(orig.maskName),
      filterFlags(orig.filterFlags),
      rasterFormat(orig.rasterFormat),
      width(orig.width),
      height(orig.height),
      depth(orig.depth),
      dataSizes(orig.dataSizes),
      paletteSize(orig.paletteSize),
      hasAlpha(orig.hasAlpha),
      mipmapCount(orig.mipmapCount),
      swizzleWidth(orig.swizzleWidth),
      swizzleHeight(orig.swizzleHeight),
      alphaDistribution(orig.alphaDistribution),
      dxtCompression(orig.dxtCompression)
    {
        if (orig.palette) {
            palette = new uint8[paletteSize * 4 * sizeof(uint8)];
            memcpy(palette, orig.palette, paletteSize * 4 * sizeof(uint8));
        } else {
            palette = nullptr;
        }
        for (uint32 i = 0; i < orig.texels.size(); i++) {
            uint32 dataSize = dataSizes[i];
            auto *newtexels = new uint8[dataSize];
            memcpy(newtexels, &orig.texels[i][0], dataSize);
            texels.push_back(newtexels);
        }
    }

    NativeTexture &NativeTexture::operator=(const NativeTexture &that) {
        if (this != &that) {
            platform = that.platform;
            name = that.name;
            maskName = that.maskName;
            filterFlags = that.filterFlags;
            rasterFormat = that.rasterFormat;
            width = that.width;
            height = that.height;
            depth = that.depth;
            dataSizes = that.dataSizes;
            paletteSize = that.paletteSize;
            hasAlpha = that.hasAlpha;
            mipmapCount = that.mipmapCount;
            swizzleWidth = that.swizzleWidth;
            swizzleHeight = that.swizzleHeight;
            dxtCompression = that.dxtCompression;
            delete[] palette;
            palette = nullptr;
            if (that.palette) {
                palette = new uint8[that.paletteSize * 4];
                memcpy(&palette[0], &that.palette[0],that.paletteSize * 4 * sizeof(uint8));
            }
            for (uint32 i = 0; i < texels.size(); i++) {
                delete[] texels[i];
                texels[i] = nullptr;
                if (that.texels[i]) {
                    texels[i] = new uint8[that.dataSizes[i]];
                    memcpy(&texels[i][0], &that.texels[i][0],that.dataSizes[i] * sizeof(uint8));
                }
            }
        }
        return *this;
    }

    NativeTexture::~NativeTexture() {
        delete[] palette;
        palette = nullptr;
        for (auto & texel : texels) {
            delete[] texel;
            texel = nullptr;
        }
    }

    string TextureDictionary::getDump() {
        string ind;
        string result;
        result += ind + "TextureDictionary {\n";
        ind += "  ";
        for (uint32 i = 0; i < texList.size(); i++) {
            NativeTexture &t = texList[i];
            result += ind + "NativeTexture " + to_string(i) + " {\n";
            ind += "  ";
            result += ind + "name: " + t.name + "\n";
            result += ind + "maskName: " + t.maskName + "\n";
            result += ind + "width: " + to_string(t.width[i]) + "\n";
            result += ind + "height: " + to_string(t.height[i]) + "\n";
            result += ind + "depth: " + to_string(t.depth) + "\n";
            result += ind + "rasterFormat: " + to_string(reinterpret_cast<long>(hex)) + to_string(t.rasterFormat) + "\n";
            ind = ind.substr(0, ind.size() - 2);
            result += ind + "}\n";
        }
        ind = ind.substr(0, ind.size() - 2);
        result += ind + "}\n";
        return result;
    }
}
