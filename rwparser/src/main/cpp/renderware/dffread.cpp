#include <cmath>

#include "renderware.h"
#include "../utils/jlogs.h"

using namespace std;

namespace rw {

	char *filename;

	/*
	 * Clump
	 */

	void Clump::read(istream& rw) {
		HeaderInfo header{};
		header.read(rw);
		READ_HEADER(CHUNK_STRUCT);
		uint32 numAtomics = readUInt32(rw);
		uint32 numLights = 0;
		if (header.length == 0xC) {
			numLights = readUInt32(rw);
			rw.seekg(4, ios::cur);
		}
		atomicList.resize(numAtomics);
		READ_HEADER(CHUNK_FRAMELIST)
		READ_HEADER(CHUNK_STRUCT)
		uint32 numFrames = readUInt32(rw);
		frameList.resize(numFrames);
		for (uint32 i = 0; i < numFrames; i++) frameList[i].readStruct(rw);
		for (uint32 i = 0; i < numFrames; i++) frameList[i].readExtension(rw);
		READ_HEADER(CHUNK_GEOMETRYLIST)
		READ_HEADER(CHUNK_STRUCT)
		uint32 numGeometries = readUInt32(rw);
		geometryList.resize(numGeometries);
		for (uint32 i = 0; i < numGeometries; i++) geometryList[i].read(rw);
		for (uint32 i = 0; i < numAtomics; i++) atomicList[i].read(rw);
		lightList.resize(numLights);
		for (uint32 i = 0; i < numLights; i++) {
			READ_HEADER(CHUNK_STRUCT)
			lightList[i].frameIndex = readInt32(rw);
			lightList[i].read(rw);
		}
		hasCollision = false;
		readExtension(rw);
	}

	void Clump::readExtension(istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_EXTENSION)
		streampos end = rw.tellg();
		end += header.length;
		while (rw.tellg() < end) {
			header.read(rw);
			switch (header.type) {
			case CHUNK_COLLISIONMODEL:
				hasCollision = true;
				colData.resize(header.length);
				rw.read((char*) &colData[0], header.length);
				break;
			default:
				rw.seekg(header.length, ios::cur);
				break;
			}
		}
	}

	string Clump::getDump(bool detailed) const {
		string result;
		string ind;
		result += ind + "Clump {\n";
		ind += "  ";
		result += ind + "numAtomics: " + to_string(atomicList.size()) + "\n";
		result += ind + "FrameList {\n";
		ind += "  ";
		result += ind + "numFrames: " + to_string(frameList.size()) + "\n";
		for (uint32 i = 0; i < frameList.size(); i++) {
			result += frameList[i].getDump(i, ind);
		}
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		result += ind + "GeometryList {\n";
		ind += "  ";
		result += ind + "numGeometries: " + to_string(geometryList.size()) + "\n";
		for (uint32 i = 0; i < geometryList.size(); i++) {
			result += geometryList[i].getDump(i, ind, detailed);
		}
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		for (uint32 i = 0; i < atomicList.size(); i++) {
			result += atomicList[i].getDump(i, ind);
		}
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		return result;
	}

	void Light::read(std::istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_LIGHT)
		READ_HEADER(CHUNK_STRUCT)
		radius = readFloat32(rw);
		rw.read((char*) &color[0], 12);
		minusCosAngle = readFloat32(rw);
		flags = readUInt16(rw);
		type = readUInt16(rw);
		READ_HEADER(CHUNK_EXTENSION);
		rw.seekg(header.length, ios::cur);
	}

	/*
	 * Atomic
	 */

	void Atomic::read(istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_ATOMIC)
		READ_HEADER(CHUNK_STRUCT)
		frameIndex = readUInt32(rw);
		geometryIndex = readUInt32(rw);
		rw.seekg(8, ios::cur);
		readExtension(rw);
	}

	void Atomic::readExtension(istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_EXTENSION)
		streampos end = rw.tellg();
		end += header.length;
		while (rw.tellg() < end) {
			header.read(rw);
			switch (header.type) {
			case CHUNK_RIGHTTORENDER:
				hasRightToRender = true;
				rightToRenderVal1 = readUInt32(rw);
				rightToRenderVal2 = readUInt32(rw);
				break;
			case CHUNK_PARTICLES:
				hasParticles = true;
				particlesVal = readUInt32(rw);
				break;
			case CHUNK_MATERIALEFFECTS:
				hasMaterialFx = true;
				materialFxVal = readUInt32(rw);
				break;
			case CHUNK_PIPELINESET:
				hasPipelineSet = true;
				pipelineSetVal = readUInt32(rw);
				break;
			default:
				rw.seekg(header.length, ios::cur);
				break;
			}
		}
	}

	string Atomic::getDump(uint32 index, string ind) const {
		string result;
		result += ind + "Atomic " + to_string(index) + " {\n";
		ind += "  ";
		result += ind + "frameIndex: " + to_string(frameIndex) + "\n";
		result += ind + "geometryIndex: " + to_string(geometryIndex) + "\n";
		if (hasRightToRender) {
			result += to_string(reinterpret_cast<long>(hex));
			result += ind + "Right to Render {\n";
			result += ind + ind + "val1: " + to_string(rightToRenderVal1) + "\n";
			result += ind + ind + "val2: " + to_string(rightToRenderVal2) + "\n";
			result += ind + "}\n";
			result += to_string(reinterpret_cast<long>(dec));
		}
		if (hasParticles) result += ind + "particlesVal: " + to_string(particlesVal) + "\n";
		if (hasPipelineSet) result += ind + "pipelineSetVal: " + to_string(pipelineSetVal) + "\n";
		if (hasMaterialFx) result += ind + "materialFxVal: " + to_string(materialFxVal) + "\n";
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		return result;
	}

	Atomic::Atomic() : frameIndex(-1), geometryIndex(-1), hasRightToRender(false),
	  rightToRenderVal1(0), rightToRenderVal2(0), hasParticles(false),
	  particlesVal(0), hasPipelineSet(false), pipelineSetVal(0),
	  hasMaterialFx(false), materialFxVal(0) {}

	/*
	 * Frame
	 */

	void Frame::readStruct(istream &rw) {
		rw.read((char *) rotationMatrix, 9 * sizeof(float32));
		rw.read((char *) position, 3 * sizeof(float32));
		parent = readInt32(rw);
		rw.seekg(4, ios::cur);
	}

	void Frame::readExtension(istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_EXTENSION)
		streampos end = rw.tellg();
		end += header.length;
		while (rw.tellg() < end) {
			header.read(rw);
			switch (header.type) {
			case CHUNK_FRAME: {
				char *buffer = new char[header.length+1];
				rw.read(buffer, header.length);
				buffer[header.length] = '\0';
				name = buffer;
				delete[] buffer;
				break;
			}
			case CHUNK_HANIM:
				hasHAnim = true;
				hAnimUnknown1 = readUInt32(rw);
				hAnimBoneId = readInt32(rw);;
				hAnimBoneCount = readUInt32(rw);
				if (hAnimBoneCount != 0) {
					hAnimUnknown2 = readUInt32(rw);
					hAnimUnknown3 = readUInt32(rw);
				}
				for (uint32 i = 0; i < hAnimBoneCount; i++) {
					hAnimBoneIds.push_back(readInt32(rw));
					hAnimBoneNumbers.push_back(readUInt32(rw));
					uint32 flag = readUInt32(rw);
					if ((flag& ~ 0x3) != 0) {
						__android_log_print(ANDROID_LOG_INFO, MODULE_TAG, "%u", flag);
					}
					hAnimBoneTypes.push_back(flag);
				}
				break;
			default:
				rw.seekg(header.length, ios::cur);
				break;
			}
		}
	}

	string Frame::getDump(uint32 index, string ind) const {
		string result;
		result += ind + "Frame " + to_string(index) + " {\n";
		ind += "  ";
		result += ind + "rotationMatrix: ";
		for (float i : rotationMatrix) result += to_string(i) + " ";
		result += "\n";
		result += ind + "position: ";
		for (float i : position) result += to_string(i) + " ";
		result += "\n";
		result += ind + "parent: " + to_string(parent) + "\n";
		result += ind + "name: " + name + "\n";
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		return result;
	}

	Frame::Frame() : parent(-1), hasHAnim(false), hAnimUnknown1(0), hAnimBoneId(-1),
	  hAnimBoneCount(0), hAnimUnknown2(0), hAnimUnknown3(0) {
		for (int i = 0; i < 3; i++) {
			position[i] = 0.0f;
			for (int j = 0; j < 3; j++) rotationMatrix[i * 3 + j] = (i == j) ? 1.0f : 0.0f;
		}
	}

	/*
	 * Geometry
	 */

	void Geometry::read(istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_GEOMETRY)
		READ_HEADER(CHUNK_STRUCT)
		flags = readUInt16(rw);
		numUVs = readUInt8(rw);
		if (flags & FLAGS_TEXTURED) numUVs = 1;
		hasNativeGeometry = readUInt8(rw);
		uint32 triangleCount = readUInt32(rw);
		vertexCount = readUInt32(rw);
		rw.seekg(4, ios::cur);
		if (header.version < 0x34000) rw.seekg(12, ios::cur);
		if (!hasNativeGeometry) {
			if (flags & FLAGS_PRELIT) {
				vertexColors.resize(4 * vertexCount);
				rw.read((char *) (&vertexColors[0]),4 * vertexCount * sizeof(uint8));
			}
			if (flags & FLAGS_TEXTURED) {
				texCoords[0].resize(2 * vertexCount);
				rw.read((char *) (&texCoords[0][0]),2 * vertexCount * sizeof(float32));
			}
			if (flags & FLAGS_TEXTURED2) {
				for (uint32 i = 0; i < numUVs; i++) {
					texCoords[i].resize(2 * vertexCount);
					rw.read((char *) (&texCoords[i][0]),2 * vertexCount * sizeof(float32));
				}
			}
			faces.resize(4*triangleCount);
			rw.read((char *) (&faces[0]), 4 * triangleCount*sizeof(uint16));
		}
		rw.read((char *) (boundingSphere), 4 * sizeof(float32));
		hasPositions = readUInt32(rw);
		hasNormals = readUInt32(rw);
		hasPositions = 1;
		hasNormals = (flags & FLAGS_NORMALS) ? 1 : 0;
		if (!hasNativeGeometry) {
			vertices.resize(3 * vertexCount);
			rw.read((char *) (&vertices[0]), 3 * vertexCount * sizeof(float32));
			if (flags & FLAGS_NORMALS) {
				normals.resize(3*vertexCount);
				rw.read((char *) (&normals[0]),3 * vertexCount * sizeof(float32));
			}
		}
		READ_HEADER(CHUNK_MATLIST)
		READ_HEADER(CHUNK_STRUCT)
		uint32 numMaterials = readUInt32(rw);
		rw.seekg(numMaterials * 4, ios::cur);
		materialList.resize(numMaterials);
		for (uint32 i = 0; i < numMaterials; i++) materialList[i].read(rw);
		readExtension(rw);
	}

	void Geometry::readExtension(istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_EXTENSION)
		streampos end = rw.tellg();
		end += header.length;
		while (rw.tellg() < end) {
			header.read(rw);
			switch (header.type) {
			case CHUNK_BINMESH: {
				faceType = readUInt32(rw);
				uint32 numSplits = readUInt32(rw);
				numIndices = readUInt32(rw);
				splits.resize(numSplits);
				bool hasData = header.length > 12 + numSplits * 8;
				for (uint32 i = 0; i < numSplits; i++) {
					uint32 indices = readUInt32(rw);
					splits[i].matIndex = readUInt32(rw);
					splits[i].indices.resize(indices);
					if (hasData) {
						if (hasNativeGeometry) {
							for (uint32 j = 0; j < indices; j++) splits[i].indices[j] =readUInt16(rw);
						} else {
							for (uint32 j = 0; j < indices; j++) splits[i].indices[j] = readUInt32(rw);
						}
					}
				}
				break;
			} case CHUNK_NATIVEDATA: {
				streampos beg = rw.tellg();
				uint32 size = header.length;
				uint32 build = header.build;
				header.read(rw);
				if (header.build == build && header.type == CHUNK_STRUCT) {
					uint32 platform = readUInt32(rw);
					rw.seekg(beg, ios::beg);
					if (platform == PLATFORM_PS2) {
						readPs2NativeData(rw);
					} else if (platform == PLATFORM_XBOX) {
						readXboxNativeData(rw);
					} else {
						string logResult = "unknown platform " + to_string(platform);
						__android_log_print(ANDROID_LOG_WARN, MODULE_TAG, "%s", logResult.c_str());
					}
				} else {
					rw.seekg(beg, ios::beg);
					readOglNativeData(rw, size);
				}
				break;
			}
			case CHUNK_MESHEXTENSION: {
				hasMeshExtension = true;
				meshExtension = new MeshExtension;
				meshExtension->unknown = readUInt32(rw);
				readMeshExtension(rw);
				break;
			} case CHUNK_NIGHTVERTEXCOLOR: {
				hasNightColors = true;
				nightColorsUnknown = readUInt32(rw);
				if (!nightColors.empty()) {
					rw.seekg(header.length - sizeof(uint32),ios::cur);
				} else {
					if (nightColorsUnknown != 0) {
						nightColors.resize(header.length - 4);
						rw.read((char *) (&nightColors[0]), header.length - 4);
					}
				}
				break;
			} case CHUNK_MORPH: {
				hasMorph = true;
				readUInt32(rw);
				break;
			} case CHUNK_SKIN: {
				if (hasNativeGeometry) {
					streampos beg = rw.tellg();
					rw.seekg(0x0c, ios::cur);
					uint32 platform = readUInt32(rw);
					rw.seekg(beg, ios::beg);
					if (platform == PLATFORM_OGL || platform == PLATFORM_PS2) {
						hasSkin = true;
						readNativeSkinMatrices(rw);
					} else if (platform == PLATFORM_XBOX) {
						hasSkin = true;
						readXboxNativeSkin(rw);
					} else {
						string logResult = "skin: unknown platform " + to_string(platform);
						__android_log_print(ANDROID_LOG_WARN, MODULE_TAG, "%s", logResult.c_str());
						rw.seekg(header.length, ios::cur);
					}
				} else {
					hasSkin = true;
					boneCount = readUInt8(rw);
					specialIndexCount = readUInt8(rw);
					unknown1 = readUInt8(rw);
					unknown2 = readUInt8(rw);
					specialIndices.resize(specialIndexCount);
					rw.read((char *) (&specialIndices[0]),specialIndexCount * sizeof(uint8));
					vertexBoneIndices.resize(vertexCount);
					rw.read((char *) (&vertexBoneIndices[0]),vertexCount * sizeof(uint32));
					vertexBoneWeights.resize(vertexCount * 4);
					rw.read((char *) (&vertexBoneWeights[0]),vertexCount * 4 * sizeof(float32));
					inverseMatrices.resize(boneCount * 16);
					for (uint32 i = 0; i < boneCount; i++) {
						if (specialIndexCount == 0) rw.seekg(4, ios::cur);
						rw.read((char *) (&inverseMatrices[i * 0x10]),0x10 * sizeof(float32));
					}
					if (specialIndexCount != 0) rw.seekg(0x0C, ios::cur);
				}
				break;
			}
			case CHUNK_ADCPLG:
				rw.seekg(header.length, ios::cur);
				break;
			case CHUNK_2DFX:
				has2dfx = true;
				twodfxData.resize(header.length);
				rw.read((char *) & twodfxData[0], header.length);
				break;
			default:
				rw.seekg(header.length, ios::cur);
				break;
			}
		}
	}

	void Geometry::readNativeSkinMatrices(istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_STRUCT)
		uint32 platform = readUInt32(rw);
		if (platform != PLATFORM_PS2 && platform != PLATFORM_OGL) {
			cerr << "error: native skin not in ps2 or ogl format\n";
			return;
		}
		boneCount = readUInt8(rw);
		specialIndexCount = readUInt8(rw);
		unknown1 = readUInt8(rw);
		unknown2 = readUInt8(rw);
		specialIndices.resize(specialIndexCount);
		rw.read((char *) (&specialIndices[0]),specialIndexCount * sizeof(uint8));
		inverseMatrices.resize(boneCount * 0x10);
		for (uint32 i = 0; i < boneCount; i++) rw.read((char *) (&inverseMatrices[i * 0x10]),0x10 * sizeof(float32));
		if (specialIndexCount != 0) rw.seekg(0x1C, ios::cur);
	}

	void Geometry::readMeshExtension(istream &rw) const {
		if (meshExtension->unknown == 0) return;
		rw.seekg(0x4, ios::cur);
		uint32 vxCount = readUInt32(rw);
		rw.seekg(0xC, ios::cur);
		uint32 faceCount = readUInt32(rw);
		rw.seekg(0x8, ios::cur);
		uint32 materialCount = readUInt32(rw);
		rw.seekg(0x10, ios::cur);
		meshExtension->vertices.resize(3*vxCount);
		rw.read((char *) (&meshExtension->vertices[0]),3 * vxCount * sizeof(float32));
		meshExtension->texCoords.resize(2 * vxCount);
		rw.read((char *) (&meshExtension->texCoords[0]),2 * vxCount * sizeof(float32));
		meshExtension->vertexColors.resize(4 * vxCount);
		rw.read((char *) (&meshExtension->vertexColors[0]),4 * vxCount * sizeof(uint8));
		meshExtension->faces.resize(3 * faceCount);
		rw.read((char *) (&meshExtension->faces[0]),3 * faceCount * sizeof(uint16));
		meshExtension->assignment.resize(faceCount);
		rw.read((char *) (&meshExtension->assignment[0]),faceCount * sizeof(uint16));
		meshExtension->textureName.resize(materialCount);
		meshExtension->maskName.resize(materialCount);
		char buffer[0x20];
		for (uint32 i = 0; i < materialCount; i++) {
			rw.read(buffer, 0x20);
			meshExtension->textureName[i] = buffer;
		}
		for (uint32 i = 0; i < materialCount; i++) {
			rw.read(buffer, 0x20);
			meshExtension->maskName[i] = buffer;
		}
		for (uint32 i = 0; i < materialCount; i++) {
			meshExtension->unknowns.push_back(readFloat32(rw));
			meshExtension->unknowns.push_back(readFloat32(rw));
			meshExtension->unknowns.push_back(readFloat32(rw));
		}
	}

	string Geometry::getDump(uint32 index, string ind, bool detailed) const {
		string result;
		result += ind + "Geometry " + to_string(index) + " {\n";
		ind += "  ";
		result += ind + "flags: " + to_string(reinterpret_cast<long>(hex)) + to_string(flags) + "\n";
		result += ind + "numUVs: " + to_string(reinterpret_cast<long>(dec)) + to_string(numUVs) + "\n";
		result += ind + "hasNativeGeometry: " + to_string(hasNativeGeometry) + "\n";
		result += ind + "triangleCount: " + to_string(faces.size() / 4) + "\n";
		result += ind + "vertexCount: " + to_string(vertexCount) + "\n";;
		if (flags & FLAGS_PRELIT) {
			result += ind + "vertexColors {\n";
			ind += "  ";
			if (!detailed) {
				result += ind + "skipping\n";
			} else {
				for (uint32 i = 0; i < vertexColors.size() / 4; i++) {
					result += ind + to_string(int(vertexColors[i * 4 + 0])) + ", "
						+ to_string(int(vertexColors[i * 4 + 1])) + ", "
						+ to_string(int(vertexColors[i * 4 + 2])) + ", "
						+ to_string(int(vertexColors[i * 4 + 3])) + "\n";
				}
			}
			ind = ind.substr(0, ind.size() - 2);
			result += ind + "}\n";
		}
		if (flags & FLAGS_TEXTURED) {
			result += ind + "texCoords {\n";
			ind += "  ";
			if (!detailed) {
				result += ind + "skipping\n";
			} else {
				for (uint32 i = 0; i < texCoords[0].size() / 2; i++) {
					result += ind + to_string(texCoords[0][i * 2 + 0]) + ", "
						+ to_string(texCoords[0][i * 2 + 1]) + "\n";
				}
			}
			ind = ind.substr(0, ind.size() - 2);
			result += ind + "}\n";
		}
		if (flags & FLAGS_TEXTURED2) {
			for (uint32 j = 0; j < numUVs; j++) {
				result += ind + "texCoords " + to_string(j) + " {\n";
				ind += "  ";
				if (!detailed) {
					result += ind + "skipping\n";
				} else {
					for (uint32 i = 0; i < texCoords[j].size() / 2; i++) {
						result += ind + to_string(texCoords[j][i * 2 + 0]) + ", "
							+ to_string(texCoords[j][i * 2 + 1]) + "\n";
					}
				}
				ind = ind.substr(0, ind.size() - 2);
				result += ind + "}\n";
			}
		}
		result += ind + "faces {\n";
		ind += "  ";
		if (!detailed) {
			result += ind + "skipping\n";
		} else {
			for (uint32 i = 0; i < faces.size() / 4; i++) {
				result += ind + to_string(faces[i * 4 + 0]) + ", "
					+ to_string(faces[i * 4 + 1]) + ", "
					+ to_string(faces[i * 4 + 2]) + ", "
					+ to_string(faces[i * 4 + 3]) + "\n";
			}
		}
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		result += ind + "boundingSphere: ";
		for (float i : boundingSphere) result += to_string(i) + " ";
		result += "\n";
		result += ind + "hasPositions: " + to_string(hasPositions) + "\n";
		result += ind + "hasNormals: " + to_string(hasNormals) + "\n";
		result += ind + "vertices {\n";
		ind += "  ";
		if (!detailed) {
			result += ind + "skipping\n";
		} else {
			for (uint32 i = 0; i < vertices.size() / 3; i++) {
				result += ind + to_string(vertices[i * 3 + 0]) + ", "
					+ to_string(vertices[i * 3 + 1]) + ", "
					+ to_string(vertices[i * 3 + 2]) + "\n";
			}
		}
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		if (flags & FLAGS_NORMALS) {
			result += ind + "normals {\n";
			ind += "  ";
			if (!detailed) {
				result += ind + "skipping\n";
			} else {
				for (uint32 i = 0; i < normals.size() / 3; i++)
					result += ind + to_string(normals[i * 3 + 0]) + ", "
						+ to_string(normals[i * 3 + 1]) + ", "
						+ to_string(normals[i * 3 + 2]) + "\n";
			}
			ind = ind.substr(0, ind.size() - 2);
			result += ind + "}\n";
		}
		result += ind + "BinMesh {\n";
		ind += "  ";
		result += ind + "faceType: " + to_string(faceType) + "\n";
		result += ind + "numIndices: " + to_string(numIndices) + "\n";
		for (uint32 i = 0; i < splits.size(); i++) {
			result += ind + "Split " + to_string(i) + " {\n";
			ind += "  ";
			result += ind + "matIndex: " + to_string(splits[i].matIndex) + "\n";
			result += ind + "numIndices: " + to_string(splits[i].indices.size()) + "\n";
			result += ind + "indices {\n";
			if (!detailed) {
				result += ind + " skipping\n";
			} else {
				for (unsigned int indice : splits[i].indices) {
					result += ind + " " + to_string(indice) + "\n";
				}
			}
			result += ind + "}\n";
			ind = ind.substr(0, ind.size() - 2);
			result += ind + "}\n";
		}
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		result += ind + "MaterialList {\n";
		ind += "  ";
		result += ind + "numMaterials: " + to_string(materialList.size()) + "\n";
		for (uint32 i = 0; i < materialList.size(); i++) {
			result += materialList[i].getDump(i, ind);
		}
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		return result;
	}

	Geometry::Geometry() : flags(0), numUVs(0), hasNativeGeometry(false), vertexCount(0),
	  hasNormals(false), faceType(0), numIndices(0), hasSkin(false), boneCount(0),
	  specialIndexCount(0), unknown1(0), unknown2(0), hasMeshExtension(false),
	  meshExtension(0), hasNightColors(false), nightColorsUnknown(0),
	  has2dfx(false), hasMorph(false) {}

	Geometry::Geometry(const Geometry &orig) : flags(orig.flags), numUVs(orig.numUVs),
	  hasNativeGeometry(orig.hasNativeGeometry), vertexCount(orig.vertexCount),
	  faces(orig.faces), vertexColors(orig.vertexColors),
	  hasPositions(orig.hasPositions), hasNormals(orig.hasNormals),
	  vertices(orig.vertices), normals(orig.normals),
	  materialList(orig.materialList), faceType(orig.faceType),
	  numIndices(orig.numIndices), splits(orig.splits), hasSkin(orig.hasSkin),
	  boneCount(orig.boneCount), specialIndexCount(orig.specialIndexCount),
	  unknown1(orig.unknown1), unknown2(orig.unknown2),
	  specialIndices(orig.specialIndices),
	  vertexBoneIndices(orig.vertexBoneIndices),
	  vertexBoneWeights(orig.vertexBoneWeights),
	  inverseMatrices(orig.inverseMatrices),
	  hasMeshExtension(orig.hasMeshExtension), hasNightColors(orig.hasNightColors),
	  nightColorsUnknown(orig.nightColorsUnknown), nightColors(orig.nightColors),
	  has2dfx(orig.has2dfx), hasMorph(orig.hasMorph)
	{
		if (orig.meshExtension) {
			meshExtension = new MeshExtension(*orig.meshExtension);
		} else {
			meshExtension = 0;
		}
		for (uint32 i = 0; i < 8; i++) texCoords[i] = orig.texCoords[i];
		for (uint32 i = 0; i < 4; i++) boundingSphere[i] = orig.boundingSphere[i];
	}

	Geometry &Geometry::operator=(const Geometry &that) {
		if (this != &that) {
			flags = that.flags;
			numUVs = that.numUVs;
			hasNativeGeometry = that.hasNativeGeometry;
			vertexCount = that.vertexCount;
			faces = that.faces;
			vertexColors = that.vertexColors;
			for (uint32 i = 0; i < 8; i++) texCoords[i] = that.texCoords[i];
			for (uint32 i = 0; i < 4; i++) boundingSphere[i] = that.boundingSphere[i];
			hasPositions = that.hasPositions;
			hasNormals = that.hasNormals;
			vertices = that.vertices;
			normals = that.normals;
			materialList = that.materialList;
			faceType = that.faceType;
			numIndices = that.numIndices;
			splits = that.splits;
			hasSkin = that.hasSkin;
			boneCount = that.boneCount;
			specialIndexCount = that.specialIndexCount;
			unknown1 = that.unknown1;
			unknown2 = that.unknown2;
			specialIndices = that.specialIndices;
			vertexBoneIndices = that.vertexBoneIndices;
			vertexBoneWeights = that.vertexBoneWeights;
			inverseMatrices = that.inverseMatrices;
			hasMeshExtension = that.hasMeshExtension;
			delete meshExtension;
			meshExtension = nullptr;
			if (that.meshExtension) meshExtension = new MeshExtension(*that.meshExtension);
			hasNightColors = that.hasNightColors;
			nightColorsUnknown = that.nightColorsUnknown;
			nightColors = that.nightColors;
			has2dfx = that.has2dfx;
			hasMorph = that.hasMorph;
		}
		return *this;
	}

	Geometry::~Geometry() { delete meshExtension; }

	/*
	 * Material
	 */

	void Material::read(istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_MATERIAL)
		READ_HEADER(CHUNK_STRUCT)
		flags = readUInt32(rw);
		rw.read((char *) (color), 4 * sizeof(uint8));
		unknown = readUInt32(rw);;
		hasTex = readInt32(rw);
		rw.read((char *) (surfaceProps), 3 * sizeof(float32));
		if (hasTex) texture.read(rw);
		readExtension(rw);
	}

	void Material::readExtension(istream &rw) {
		HeaderInfo header{};
		char buf[32];
		READ_HEADER(CHUNK_EXTENSION)
		streampos end = rw.tellg();
		end += header.length;
		while (rw.tellg() < end) {
			header.read(rw);
			switch (header.type) {
				case CHUNK_RIGHTTORENDER:
					hasRightToRender = true;
					rightToRenderVal1 = readUInt32(rw);
					rightToRenderVal2 = readUInt32(rw);
					break;
				case CHUNK_MATERIALEFFECTS: {
					hasMatFx = true;
					matFx = new MatFx;
					matFx->type = readUInt32(rw);
					switch (matFx->type) {
						case MATFX_BUMPMAP: {
							rw.seekg(4, ios::cur);
							matFx->bumpCoefficient = readFloat32(rw);
							matFx->hasTex1 = readUInt32(rw);
							if (matFx->hasTex1) matFx->tex1.read(rw);
							matFx->hasTex2 = readUInt32(rw);
							if (matFx->hasTex2) matFx->tex2.read(rw);
							rw.seekg(4, ios::cur);
							break;
						}
						case MATFX_ENVMAP: {
							rw.seekg(4, ios::cur);
							matFx->envCoefficient = readFloat32(rw);
							matFx->hasTex1 = readUInt32(rw);
							if (matFx->hasTex1) matFx->tex1.read(rw);
							matFx->hasTex2 = readUInt32(rw);
							if (matFx->hasTex2) matFx->tex2.read(rw);
							rw.seekg(4, ios::cur);
							break;
						}
						case MATFX_BUMPENVMAP: {
							rw.seekg(4, ios::cur);
							matFx->bumpCoefficient = readFloat32(rw);
							matFx->hasTex1 = readUInt32(rw);
							if (matFx->hasTex1) matFx->tex1.read(rw);
							rw.seekg(4, ios::cur);
							rw.seekg(4, ios::cur);
							matFx->envCoefficient = readFloat32(rw);
							rw.seekg(4, ios::cur);
							matFx->hasTex2 = readUInt32(rw);
							if (matFx->hasTex2) matFx->tex2.read(rw);
							break;
						}
						case MATFX_DUAL: {
							rw.seekg(4, ios::cur);
							matFx->srcBlend = readUInt32(rw);
							matFx->destBlend = readUInt32(rw);
							matFx->hasDualPassMap = readUInt32(rw);
							if (matFx->hasDualPassMap) matFx->dualPassMap.read(rw);
							rw.seekg(4, ios::cur);
							break;
						}
						case MATFX_UVTRANSFORM: {
							rw.seekg(4, ios::cur);
							rw.seekg(4, ios::cur);
							break;
						}
						case MATFX_DUALUVTRANSFORM: {
							break;
						}
						default:
							break;
					}
					break;
				}
				case CHUNK_REFLECTIONMAT:
					hasReflectionMat = true;
					reflectionChannelAmount[0] = readFloat32(rw);
					reflectionChannelAmount[1] = readFloat32(rw);
					reflectionChannelAmount[2] = readFloat32(rw);
					reflectionChannelAmount[3] = readFloat32(rw);
					reflectionIntensity = readFloat32(rw);
					rw.seekg(4, ios::cur);
					break;
				case CHUNK_SPECULARMAT: {
					hasSpecularMat = true;
					specularLevel = readFloat32(rw);
					uint32 len = header.length - sizeof(float32) - 4;
					char *name = new char[len];
					rw.read(name, len);
					specularName = name;
					rw.seekg(4, ios::cur);
					delete[] name;
					break;
				}
				case CHUNK_UVANIMPLG:
					READ_HEADER(CHUNK_STRUCT)
					hasUVAnim = true;
					uvVal = readUInt32(rw);
					rw.read(buf, 32);
					uvName = buf;
					break;
				default:
					rw.seekg(header.length, ios::cur);
					break;
			}
		}
	}

	string Material::getDump(uint32 index, string ind) const {
		string result;
		result += ind + "Material " + to_string(index) + " {\n";
		ind += "  ";
		result += ind + "color: " + to_string(reinterpret_cast<long>(dec))
			+ to_string(int(color[0])) + " "
			+ to_string(int(color[1])) + " "
			+ to_string(int(color[2])) + " "
			+ to_string(int(color[3])) + "\n";
		result += ind + "surfaceProps: " + to_string(surfaceProps[0]) + " "
			+ to_string(surfaceProps[1]) + " "
			+ to_string(surfaceProps[2]) + "\n";
		if (hasTex) result += texture.getDump(ind);
		if (hasMatFx) result += matFx->getDump(ind);
		if (hasRightToRender) {
			result += to_string(reinterpret_cast<long>(hex));
			result += ind + "Right to Render {\n";
			result += ind + "  " + "val1: " + to_string(rightToRenderVal1) + "\n";
			result += ind + "  " + "val2: " + to_string(rightToRenderVal2) + "\n";
			result += ind + "}\n";
			result += to_string(reinterpret_cast<long>(dec));
		}
		if (hasReflectionMat) {
			result += ind + "Reflection Material {\n";
			result += ind + "  " + "amount: "
				+ to_string(reflectionChannelAmount[0]) + " "
				+ to_string(reflectionChannelAmount[1]) + " "
				+ to_string(reflectionChannelAmount[2]) + " "
				+ to_string(reflectionChannelAmount[3]) + "\n";
			result += ind + "  " + "intensity: " + to_string(reflectionIntensity) + "\n";
			result += ind + "}\n";
		}
		if (hasSpecularMat) {
			result += ind + "Specular Material {\n";
			result += ind + "  " + "level: " + to_string(specularLevel) + "\n";
			result += ind + "  " + "name: " + specularName + "\n";
			result += ind + "}\n";
		}
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		return result;
	}

	Material::Material() : flags(0), unknown(0), hasTex(false), hasRightToRender(false),
	  rightToRenderVal1(0), rightToRenderVal2(0), hasMatFx(false), matFx(0),
	  hasReflectionMat(false), reflectionIntensity(0.0f), hasSpecularMat(false),
	  specularLevel(0.0f), hasUVAnim(false)
	{
		for (unsigned char & i : color) i = 0;
		for (int i = 0; i < 3; i++) {
			surfaceProps[i] = 0.0f;
			reflectionChannelAmount[i] = 0.0f;
		}
	}

	Material::Material(const Material& orig) : flags(orig.flags), unknown(orig.unknown),
	  hasTex(orig.hasTex), texture(orig.texture),
	  hasRightToRender(orig.hasRightToRender),
	  rightToRenderVal1(orig.rightToRenderVal1),
	  rightToRenderVal2(orig.rightToRenderVal2),
	  hasMatFx(orig.hasMatFx), hasReflectionMat(orig.hasReflectionMat),
	  reflectionIntensity(orig.reflectionIntensity),
	  hasSpecularMat(orig.hasSpecularMat), specularLevel(orig.specularLevel),
	  specularName(orig.specularName), hasUVAnim(orig.hasUVAnim)
	{
		if (orig.matFx) {
			matFx = new MatFx(*orig.matFx);
		} else {
			matFx = nullptr;
		}
		for (uint32 i = 0; i < 4; i++) color[i] = orig.color[i];
		for (uint32 i = 0; i < 3; i++) surfaceProps[i] = orig.surfaceProps[i];
		for (uint32 i = 0; i < 4; i++) reflectionChannelAmount[i] = orig.reflectionChannelAmount[i];
	}

	Material &Material::operator=(const Material &that) {
		if (this != &that) {
			flags = that.flags;
			for (uint32 i = 0; i < 4; i++) color[i] = that.color[i];
			unknown = that.unknown;
			hasTex = that.hasTex;
			for (uint32 i = 0; i < 3; i++) surfaceProps[i] = that.surfaceProps[i];
			texture = that.texture;
			hasRightToRender = that.hasRightToRender;
			rightToRenderVal1 = that.rightToRenderVal1;
			rightToRenderVal2 = that.rightToRenderVal2;
			hasMatFx = that.hasMatFx;
			delete matFx;
			matFx = nullptr;
			if (that.matFx) matFx = new MatFx(*that.matFx);
			hasReflectionMat = that.hasReflectionMat;
			for (uint32 i = 0; i < 4; i++) reflectionChannelAmount[i] = that.reflectionChannelAmount[i];
			reflectionIntensity = that.reflectionIntensity;
			hasSpecularMat = that.hasSpecularMat;
			specularLevel = that.specularLevel;
			specularName = that.specularName;
			hasUVAnim = that.hasUVAnim;
		}
		return *this;
	}

	Material::~Material() { delete matFx; }

	string MatFx::getDump(string ind) const {
		string result;
		static const char *names[] = {
			"INVALID",
			"MATFX_BUMPMAP",
			"MATFX_ENVMAP",
			"MATFX_BUMPENVMAP",
			"MATFX_DUAL",
			"MATFX_UVTRANSFORM",
			"MATFX_DUALUVTRANSFORM"
		};
		result += ind + "MatFX {\n";
		ind += "  ";
		result += ind + "type: " + names[type] + "\n";
		if (type == MATFX_BUMPMAP || type == MATFX_BUMPENVMAP) {
			result += ind + "bumpCoefficient: " + to_string(bumpCoefficient) + "\n";
		}
		if (type == MATFX_ENVMAP || type == MATFX_BUMPENVMAP) {
			result += ind + "envCoefficient: " + to_string(envCoefficient) + "\n";
		}
		if (type == MATFX_DUAL) {
			result += ind + "srcBlend: " + to_string(srcBlend) + "\n";
			result += ind + "destBlend: " + to_string(destBlend) + "\n";
		}
		result += ind + "textures: " + to_string(hasTex1) + " "
				+ to_string(hasTex2) + " "
				+ to_string(hasDualPassMap) + "\n";
		if (hasTex1) result += tex1.getDump(ind);
		if (hasTex2) result += tex2.getDump(ind);
		if (hasDualPassMap) result += dualPassMap.getDump(ind);
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		return result;
	}

	MatFx::MatFx() : hasTex1(false), hasTex2(false), hasDualPassMap(false) {}

	/*
	 * Texture
	 */

	void Texture::read(istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_TEXTURE)
		READ_HEADER(CHUNK_STRUCT)
		filterFlags = readUInt16(rw);
		rw.seekg(2, ios::cur);
		READ_HEADER(CHUNK_STRING)
		char *buffer = new char[header.length+1];
		rw.read(buffer, header.length);
		buffer[header.length] = '\0';
		name = buffer;
		delete[] buffer;
		READ_HEADER(CHUNK_STRING)
		buffer = new char[header.length+1];
		rw.read(buffer, header.length);
		buffer[header.length] = '\0';
		maskName = buffer;
		delete[] buffer;
		readExtension(rw);
	}

	void Texture::readExtension(istream &rw) {
		HeaderInfo header{};
		READ_HEADER(CHUNK_EXTENSION)
		streampos end = rw.tellg();
		end += header.length;
		while (rw.tellg() < end) {
			header.read(rw);
			switch (header.type) {
			case CHUNK_SKYMIPMAP:
				hasSkyMipmap = true;
				rw.seekg(header.length, ios::cur);
				break;
			default:
				rw.seekg(header.length, ios::cur);
				break;
			}
		}
	}

	string Texture::getDump(std::string ind) const {
		string result;
		result += ind + "Texture {\n";
		ind += "  ";
		result += ind + "filterFlags: " + to_string(reinterpret_cast<long>(hex))
			  + to_string(filterFlags)
			  + to_string(reinterpret_cast<long>(dec)) + "\n";
		result += ind + "name: " + name + "\n";
		result += ind + "maskName: " + maskName + "\n";
		ind = ind.substr(0, ind.size() - 2);
		result += ind + "}\n";
		return result;
	}

	Texture::Texture() : filterFlags(0), name(""), maskName(""), hasSkyMipmap(false) {}
}
