#include "renderware.h"

#include <cstring>

using namespace std;

namespace rw {

	static void convertattrib(float *dst, char *data, int type, int normalized, int n) {
		unsigned char *uc;
		short *sh;
		unsigned short *ush;
		enum {
			FLOAT = 0,
			BYTE,
			UBYTE,
			SHORT,
			USHORT
		};
		switch (type) {
			case FLOAT:
				memcpy(dst, data, n * sizeof(float));
				break;
			case BYTE:
				for (int i = 0; i < n; i++) {
					dst[i] = data[i];
					if (normalized) dst[i] /= 128.0f;
				}
				break;
			case UBYTE:
				uc = (unsigned char *) data;
				for (int i = 0; i < n; i++) {
					dst[i] = uc[i];
					if (normalized) dst[i] /= 255.0f;
				}
				break;
			case SHORT:
				sh = (short *) data;
				for (int i = 0; i < n; i++) {
					dst[i] = sh[i];
					if (normalized) dst[i] /= 32768.0f;
				}
				break;
			case USHORT:
				ush = (unsigned short *) data;
				for (int i = 0; i < n; i++) {
					dst[i] = ush[i];
					if (normalized) dst[i] /= 65536.0f;
				}
				break;
			default:
				break;
		}
	}

	void Geometry::readOglNativeData(istream &rw, int size) {
		uint32 nattribs;
		uint32 *attribs, *ap;
		char *data, *vdata, *dp;
		float f[4];
		enum {
			VERTICES = 0,
			UVS,
			NORMALS,
			COLORS,
			WEIGHTS,
			INDICES
		};
		nattribs = readUInt32(rw);
		data = new char[size - sizeof(uint32)];
		rw.read(data, size - sizeof(uint32));
		attribs = (uint32 *) data;
		vdata = data + nattribs * 6 * sizeof(uint32);
		ap = attribs;
		for (uint32 i = 0; i < nattribs; i++, ap += 6) {
			dp = vdata + ap[5];
			switch (ap[0]) {
				case VERTICES:
					for (uint32 j = 0; j < vertexCount; j++) {
						convertattrib(f, dp, ap[1], ap[2], ap[3]);
						vertices.push_back(f[0]);
						vertices.push_back(f[1]);
						vertices.push_back(f[2]);
						dp += ap[4];
					}
					break;
				case UVS:
					for (uint32 j = 0; j < vertexCount; j++) {
						convertattrib(f, dp, ap[1], ap[2], ap[3]);
						texCoords[0].push_back(f[0] / 512.0f);
						texCoords[0].push_back(f[1] / 512.0f);
						dp += ap[4];
					}
					break;
				case NORMALS:
					for (uint32 j = 0; j < vertexCount; j++) {
						convertattrib(f, dp, ap[1], ap[2], ap[3]);
						normals.push_back(f[0]);
						normals.push_back(f[1]);
						normals.push_back(f[2]);
						dp += ap[4];
					}
					break;
				case COLORS:
					for (uint32 j = 0; j < vertexCount; j++) {
						convertattrib(f, dp, ap[1], ap[2], ap[3]);
						vertexColors.push_back(f[0]);
						vertexColors.push_back(f[1]);
						vertexColors.push_back(f[2]);
						vertexColors.push_back(f[3]);
						dp += ap[4];
					}
					break;
				case WEIGHTS:
					for (uint32 j = 0; j < vertexCount; j++) {
						convertattrib(f, dp, ap[1], ap[2], ap[3]);
						vertexBoneWeights.push_back(f[0]);
						vertexBoneWeights.push_back(f[1]);
						vertexBoneWeights.push_back(f[2]);
						vertexBoneWeights.push_back(f[3]);
						dp += ap[4];
					}
					break;
				case INDICES:
					for (uint32 j = 0; j < vertexCount; j++) {
						convertattrib(f, dp, ap[1], ap[2], ap[3]);
						uint32 idx = ((int) f[3] << 24) | ((int) f[2] << 16) | ((int) f[1] << 8) | (int) f[0];
						vertexBoneIndices.push_back(idx);
						dp += ap[4];
					}
					break;
			}
		}
		delete[] attribs;
	}
}
