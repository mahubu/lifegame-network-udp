#pragma once

#include <network/Sockets.hpp>

namespace network
{
	namespace event
	{
		/*
		* @brief A client associated to an event.
		*/
		struct Client
		{
			uint16_t uid{ 0 };
			sockaddr_storage address{ 0 };
		};

		/*
		* @brief An event template.
		*/
		class Event
		{
		public:
			/*
			* @return whether (or not) an event is of the given type.
			*/
			template<typename M> bool is() const
			{
				return type_ == M::STATIC_TYPE;
			}

			/*
			* @return the event as one of the given type.
			*/
			template<typename M> const M* as() const
			{
				return static_cast<const M*>(this);
			}

			/*
			* @return the client associated to the event.
			*/
			const Client& client() const { return client_; };

		protected:
			enum class Type
			{
				Connection,
				Disconnection,
				Exchange
			};

			Event(const Type type, const Client& client) : type_(type)
			{
				client_.uid = client.uid;
				std::memcpy(&client_.address, &client.address, sizeof(client.address));
			}

		private:
			Type type_;
			Client client_;

		};
	}
}
