#pragma once
#include <memory>

struct Handle {
	uint32_t id;
	std::shared_ptr<void> ref;

	void Create(uint32_t _id) {
		id = _id;
		ref = std::make_shared<int>(0);
	}

	Handle Share() {
		Handle handle;
		handle.id = id;
		handle.ref = ref;
		return handle;
	}

};

