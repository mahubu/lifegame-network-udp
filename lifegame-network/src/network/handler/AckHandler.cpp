#include "network/Sockets.hpp"
#include "network/handler/AckHandler.hpp"
#include "network/helper/BitHelper.hpp"

#include <algorithm>
#include <assert.h>
#include <numeric>

namespace network
{
	namespace handler
	{

		void AckHandler::update(uint16_t newAck, uint64_t previousAcks, bool trackLoss /*= false*/)
		{
			isLastAckNew_ = false;
			// 1.Ack is the same as latest known, but mask may vary.
			if (newAck == lastAck_)
			{
				// 1. Filtering common acks on both sides (local & remote).
				// 2. Filtering new acks from remote side.
				newAcks_ = (previousAcks_ & previousAcks) ^ previousAcks;
				// Updating previous acks on local side with remote side.
				previousAcks_ |= previousAcks;
			}

			// 2. Ack is a newer one.
			else if (helper::isNewer(newAck, lastAck_))
			{
				const auto diff = helper::difference(newAck, lastAck_);
				// Bits to shift on the mask.
				const auto shift = std::min(diff, static_cast<uint16_t>(64));
				if (trackLoss)
				{
					for (uint32_t i = 0; i < shift; ++i)
					{
						const auto packetDiffWithLastAck = 64 - i;
						const auto bitInPreviousMask = packetDiffWithLastAck - 1;
						// Lost  packets.
						if (!helper::hasBit(previousAcks_, bitInPreviousMask))
						{
							const uint16_t packetid = lastAck_ - packetDiffWithLastAck;
							losses_.push_back(packetid);
						}
					}
				}
				// Shifting mask.
				previousAcks_ <<= shift;

				const uint16_t gap = diff - 1;
				// TODO factorize with previous losses ??? ----->
				if (gap >= 64)
				{
					// A 'too wide' gap wipe the entire mask. 
					newAcks_ = 0;
					previousAcks_ = 0;
					if (trackLoss)
					{
						// Lost packets.
						for (uint32_t i = 64; i < gap; ++i)
						{
							const uint16_t packetid = lastAck_ + (i - 64) + 1;
							losses_.push_back(packetid);
						}
					}
				}
				// TODO factorize with previous losses ??? <-----
				else
				{
					// Recording ack into the mask.
					helper::setBit(previousAcks_, gap);
				}
				lastAck_ = newAck;
				isLastAckNew_ = true;
				newAcks_ = (previousAcks_ & previousAcks) ^ previousAcks;
				previousAcks_ |= previousAcks;
			}

			// 3. Ack is a older one.
			else
			{
				const auto diff = helper::difference(lastAck_, newAck);
				// Ack is not too old and unknown from mask.
				if (diff <= 64)
				{
					// Shifting old mask.
					auto oldAcks = previousAcks << diff;
					// Recording old ack into the old mask.
					const auto ackBitInMask = diff - 1;
					helper::setBit(oldAcks, ackBitInMask);
					// Updating ack masks with old ones.
					newAcks_ = (previousAcks_ & oldAcks) ^ oldAcks;
					previousAcks_ |= oldAcks;
				}
			}
		}

		bool AckHandler::isAcked(uint16_t ack) const
		{
			if (ack == lastAck_)
			{
				return true;
			}
			if (helper::isNewer(ack, lastAck_))
			{
				return false;
			}
			const auto diff = helper::difference(lastAck_, ack);
			if (diff > 64)
			{
				return false;
			}
			const uint8_t position = diff - 1;
			return helper::hasBit(previousAcks_, position);
		}

		bool AckHandler::isNewlyAcked(uint16_t ack) const
		{
			if (ack == lastAck_)
			{
				return isLastAckNew_;
			}
			if (helper::isNewer(ack, lastAck_))
			{
				return false;
			}
			const auto diff = helper::difference(lastAck_, ack);
			if (diff > 64)
			{
				return false;
			}
			const uint8_t position = diff - 1;
			return helper::hasBit(newAcks_, position);
		}

		uint16_t AckHandler::lastAck() const
		{
			return lastAck_;
		}

		uint64_t AckHandler::previousAcks() const
		{
			return previousAcks_;
		}

		std::vector<uint16_t> AckHandler::newAcks() const
		{
			std::vector<uint16_t> newAcks;
			newAcks.reserve(65);
			for (uint8_t i = 64; i != 0; --i)
			{
				const uint8_t bitToCheck = i - 1;
				if (helper::hasBit(newAcks_, bitToCheck))
				{
					const uint16_t id = lastAck_ - i;
					newAcks.push_back(id);
				}
			}
			if (isLastAckNew_)
			{
				newAcks.push_back(lastAck_);
			}
			return newAcks;
		}

		std::vector<uint16_t> AckHandler::losses()
		{
			std::vector<uint16_t> l(losses_);
			losses_.clear();
			return l;
		}

	}
}