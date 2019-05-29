#pragma once

#include <cstdint>
#include <vector>

namespace network
{
	namespace handler
	{
		/*
		* @brief An handler for tracking acknowledgement for sended or received packets.
		*/
		class AckHandler
		{
		public:
			AckHandler() = default;
			AckHandler(const AckHandler&) = default;
			AckHandler& operator=(const AckHandler&) = default;
			AckHandler(AckHandler&&) = default;
			AckHandler& operator=(AckHandler&&) = default;
			~AckHandler() = default;

			/*
			* @brief Update the handler state with latest acknowledgement & previous acknowledgements from the remote peer.
			*
			* @param newAck latest acknowledgement from the remote peer.
			* @param previousAcks previous acknowledgements from the remote peer.
			* @param trackLoss whether (or not) lost packets must be tracked.
			*/
			void update(uint16_t newAck, uint64_t previousAcks, bool trackLoss = false);

			/*
			* @brief Check if a potential acknowledgement has been actually acked.
			*
			* @param ack the acknowledgement to check.
			*
			* @return whether (nor not) the acknowledgement has been actually acked.
			*/
			bool isAcked(uint16_t ack) const;

			/*
			* @brief Check if a potential acknowledgement has been recently acked.
			*
			* @param ack the acknowledgement to check.
			*
			* @return whether (nor not) the acknowledgement has been recently acked.
			*/
			bool isNewlyAcked(uint16_t ack) const;

			/*
			* @return the latest known acknowledgement.
			*/
			uint16_t lastAck() const;

			/*
			* @return the mask of previous known acknowledgements.
			*/
			uint64_t previousAcks() const;

			/*
			* @return the latest acknowledgements.
			*/
			std::vector<uint16_t> newAcks() const;

			/*
			* @return the latest losses (clear losses on call !).
			*/
			std::vector<uint16_t> losses(); // TODO clear loses !!! -> another method ??? -> pollLosses() ????

		private:
			uint16_t lastAck_ = -1; // -1 stands for 'no last ack known'.
			uint64_t previousAcks_ = -1;  // -1 stands for 'no previous acks known'.
			uint64_t newAcks_ = 0; // 0 stand for 'no new acks'.
			std::vector<uint16_t> losses_;
			bool isLastAckNew_ = false;
		};
	}
}
