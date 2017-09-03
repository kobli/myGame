#ifndef PERLINMULTIOCTAVE_HPP_17_08_05_13_53_38
#define PERLINMULTIOCTAVE_HPP_17_08_05_13_53_38 
#include "perlin.hpp"

class PerlinMultiOctave {
	public:
		struct Octave {
			float frequency;
			float amplitude;
		};

		class Octaves: private std::vector<Octave> {
			private:
				using BaseT = std::vector<Octave>;
			public:
				Octaves(float maxFrequency, float amplitude, unsigned octavesC = 1, float persistence = 0.5);
				void addOctave(Octave o);
				float maxFrequency() const;

				using BaseT::begin;
				using BaseT::end;
				using BaseT::size;
		};

		PerlinMultiOctave(unsigned sizeX, unsigned sizeY, Octaves octaves, unsigned seed = 1);
		float val(unsigned x, unsigned y);

		void setSeed(unsigned seed);
		unsigned getSeed();

	private:
		Perlin _perlin;
		unsigned _sizeX;
		unsigned _sizeY;
		Octaves _octaves;
};


#endif /* PERLINMULTIOCTAVE_HPP_17_08_05_13_53_38 */
