#ifndef HEIGHTMAPMESH_HPP_17_08_25_21_23_16
#define HEIGHTMAPMESH_HPP_17_08_25_21_23_16 
#include "main.hpp"

class HeightmapMesh
{
	public:
		typedef std::function<video::SColor(f32 x, f32 y, f32 z, vec3f normal)> ColoringFunc;
		scene::SMesh* Mesh;

		HeightmapMesh(): Mesh(nullptr), _vertexDensity{4}
		{
			Mesh = new scene::SMesh();
		}

		~HeightmapMesh()
		{
			Mesh->drop();
		}

		// Unless the heightmap is small, it won't all fit into a single
		// SMeshBuffer. This function chops it into pieces and generates a
		// buffer from each one.

		void init(const Terrain& t, ColoringFunc cf, video::IVideoDriver *driver)
		{
			const u32 mp = driver -> getMaximalPrimitiveCount();
			unsigned h = t.size().Y;

			const u32 sw = mp / (6*h*_vertexDensity); // the width of each piece

			u32 i=0;
			for(u32 y0 = 0; y0 < h*_vertexDensity; y0 += sw)
			{
				u16 y1 = y0 + sw;
				if(y1 >= h*_vertexDensity)
					y1 = h*_vertexDensity - 1;
				addstrip(t, cf, y0, y1, i);
				++i;
			}
			if(i < Mesh->getMeshBufferCount())
			{
				for(u32 j = i; j < Mesh->getMeshBufferCount(); ++j)
				{
					Mesh->getMeshBuffer(j)->drop();
				}
				Mesh->MeshBuffers.erase(i, Mesh->getMeshBufferCount()-i);
			}
			Mesh->setDirty();
			Mesh->recalculateBoundingBox();
		}

		void addstrip(const Terrain& t, ColoringFunc cf, u16 y0, u16 y1, u32 bufNum)
		{
			unsigned w = (t.size().X-1)*_vertexDensity+1;
			unsigned h = (t.size().Y-1)*_vertexDensity+1;

			scene::SMeshBuffer *buf = 0;
			if (bufNum<Mesh->getMeshBufferCount())
			{
				buf = (scene::SMeshBuffer*)Mesh->getMeshBuffer(bufNum);
			}
			else
			{
				// create new buffer
				buf = new scene::SMeshBuffer();
				Mesh->addMeshBuffer(buf);
				// to simplify things we drop here but continue using buf
				buf->drop();
			}
			buf->Vertices.set_used((1 + y1 - y0) * w);

			u32 i=0;
			for (u16 y = y0; y <= y1; ++y)
			{
				for (u16 x = 0; x < w; ++x)
				{
					const f32 z = t.heightAt(x/float(_vertexDensity), y/float(_vertexDensity));
					const f32 xx = (f32)x/(f32)w;
					const f32 yy = (f32)y/(f32)h;

					video::S3DVertex& v = buf->Vertices[i++];
					v.Pos.set(x/float(_vertexDensity), z, y/float(_vertexDensity));
					v.Normal.set(t.normalAt(x/float(_vertexDensity), y/float(_vertexDensity)));
					v.Color=cf(xx, yy, z, v.Normal);
					v.TCoords.set(xx, yy);
				}
			}

			buf->Indices.set_used(6 * (w - 1) * (y1 - y0));
			i=0;
			for(u16 y = y0; y < y1; ++y)
			{
				for(u16 x = 0; x < w - 1; ++x)
				{
					const u16 n = (y-y0) * w + x;
					buf->Indices[i]=n;
					buf->Indices[++i]=n + w;
					buf->Indices[++i]=n + w + 1;
					buf->Indices[++i]=n + w + 1;
					buf->Indices[++i]=n + 1;
					buf->Indices[++i]=n;
					++i;
				}
			}

			buf->recalculateBoundingBox();
		}
	private:
		unsigned _vertexDensity;
};
#endif /* HEIGHTMAPMESH_HPP_17_08_25_21_23_16 */
