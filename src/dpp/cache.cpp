#include <dpp/discord.h>
#include <mutex>
#include <iostream>
#include <variant>

namespace dpp {

user_map users;
channel_map channels;
guild_map guilds;
role_map roles;
std::unordered_map<managed*, time_t> deletion_queue;

std::mutex user_mutex;
std::mutex channel_mutex;
std::mutex guild_mutex;
std::mutex role_mutex;
std::mutex deletion_mutex;

/* Because other threads and systems may run for a short while after an event is received, we don't immediately
 * delete pointers when objects are replaced. We put them into a queue, and periodically delete pointers in the
 * queue.
 */
void garbage_collection() {
	time_t now = time(NULL);
	bool repeat = false;
	std::lock_guard<std::mutex> delete_lock(deletion_mutex);
	do {
		for (auto g = deletion_queue.begin(); g != deletion_queue.end(); ++g) {
			if (now > g->second + 60) {
				delete g->first;
				repeat = true;
				break;
			}
		}
	} while (repeat);
}

void store_guild(guild* g) {
	std::lock_guard<std::mutex> lock(guild_mutex);
	auto existing = guilds.find(g->id);
	if (existing == guilds.end()) {
		guilds[g->id] = g;
	} else {
		/* Flag old pointer for deletion and replace */
		std::lock_guard<std::mutex> delete_lock(deletion_mutex);
		deletion_queue[existing->second] = time(NULL);
		guilds[g->id] = g;
	}
}

guild* find_guild(snowflake id) {
	std::lock_guard<std::mutex> lock(guild_mutex);
	auto r = guilds.find(id);
	if (r != guilds.end()) {
		return r->second;
	}
	return nullptr;
}

void store_user(user * u) {
	std::lock_guard<std::mutex> lock(user_mutex);
	auto existing = users.find(u->id);
	if (existing == users.end()) {
		users[u->id] = u;
	} else {
		/* Flag old pointer for deletion and replace */
		std::lock_guard<std::mutex> delete_lock(deletion_mutex);
		deletion_queue[existing->second] = time(NULL);
		users[u->id] = u;
	}

}

user* find_user(snowflake id) {
	std::lock_guard<std::mutex> lock(user_mutex);
	auto r = users.find(id);
	if (r != users.end()) {
		return r->second;
	}
	return nullptr;
}

void store_channel(channel* c) {
	std::lock_guard<std::mutex> lock(channel_mutex);
	auto existing = channels.find(c->id);
	if (existing == channels.end()) {
		channels[c->id] = c;
	} else {
		/* Flag old pointer for deletion and replace */
		std::lock_guard<std::mutex> delete_lock(deletion_mutex);
		deletion_queue[existing->second] = time(NULL);
		channels[c->id] = c;
	}
}

channel* find_channel(snowflake id) {
	std::lock_guard<std::mutex> lock(channel_mutex);
	auto r = channels.find(id);
	if (r != channels.end()) {
		return r->second;
	}
	return nullptr;
}

void store_role(role* r) {
	std::lock_guard<std::mutex> lock(role_mutex);
	auto existing = roles.find(r->id);
	if (existing == roles.end()) {
		roles[r->id] = r;
	} else {
		/* Flag old pointer for deletion and replace */
		std::lock_guard<std::mutex> delete_lock(deletion_mutex);
		deletion_queue[existing->second] = time(NULL);
		roles[r->id] = r;
	}

}

role* find_role(snowflake id) {
	std::lock_guard<std::mutex> lock(role_mutex);
	auto r = roles.find(id);
	if (r != roles.end()) {
		return r->second;
	}
	return nullptr;
}


};
