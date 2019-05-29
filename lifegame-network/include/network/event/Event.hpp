#pragma once

#include <network/Sockets.hpp>

namespace network
{
	namespace event
	{
		/*
		* @brief An event template.
		*/
		class Event
		{
		public:
			/*
			* @return whether (or not) an event is of the given type.
			*/
			template<class M> bool is() const
			{
				return type_ == M::STATIC_TYPE;
			}
			/*
			* @return the event as one of the given type.
			*/
			template<class M> const M* as() const
			{
				return static_cast<const M*>(this);
			}

			const sockaddr_storage& client() const { return client_; };

		protected:
			enum class Type
			{
				Connection,
				Disconnection,
				Exchange
			};

			Event(const Type type, const sockaddr_storage& client) : type_(type)/*, client_(client)*/
			{
				memcpy(&client_, &client, sizeof(client));
			}

		private:
			Type type_;
			sockaddr_storage client_;

		};
	}
}
