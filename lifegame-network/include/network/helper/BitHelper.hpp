#pragma once

#include <network/helper/BitHelper.hpp>
#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <numeric>

namespace network
{
	namespace helper
	{
		// Endianness is fixed to 'big'.
		static constexpr uint64_t UNSIGNED_LONG_BIG_ENDIAN_UNITY = 0b0000000000000000000000000000000000000000000000000000000000000001;

		inline void setBit(uint64_t& bitfield, const uint16_t n)
		{
			assert(n < 64);
			bitfield |= (UNSIGNED_LONG_BIG_ENDIAN_UNITY << n);
		}

		inline void unsetBit(uint64_t & bitfield, const uint16_t n)
		{
			assert(n < 64);
			bitfield &= (~(UNSIGNED_LONG_BIG_ENDIAN_UNITY << n));
		}

		inline bool hasBit(const uint64_t bitfield, const uint16_t n)
		{
			assert(n < 64);
			return (bitfield & (UNSIGNED_LONG_BIG_ENDIAN_UNITY << n)) != 0;
		}

		// 'Identifier re-use' detection.
		static constexpr uint16_t OVERFLOW_DETECTION_LIMIT = std::numeric_limits<uint16_t>::max() / 10;

		inline bool isNewer(uint16_t newId, uint16_t latestId)
		{
			if (newId == latestId)
			{
				return false;
			}

			// Standard 'expected' case.
			// New value is greater than latest value and values are close.
			if (newId > latestId && newId - latestId <= OVERFLOW_DETECTION_LIMIT)
			{
				return true;
			}
			// Overflow 'loop' case : values have reached 'unsigned short' maximum (= 65535) and restarted from 'unsigned short' minimum (= 0).
			// New value is smaller than latest value and values are way apart.
			if (newId < latestId && latestId - newId > OVERFLOW_DETECTION_LIMIT)
			{
				return true;
			}
			return false;
		}

		inline uint16_t difference(uint16_t newId, uint16_t latestId)
		{
			if (newId == latestId)
			{
				return 0;
			}

			assert(isNewer(newId, latestId));
			// Standard case.
			if (newId > latestId && newId - latestId <= OVERFLOW_DETECTION_LIMIT)
			{
				return newId - latestId;
			}
			// Overflow case.
			else
			{
				return std::numeric_limits<uint16_t>::max() - latestId + newId + 1; // '+1' is for taking '0' into account.
			}
		}
	}
}
