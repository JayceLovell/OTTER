#include "IBuffer.h"

IBuffer::IBuffer(BufferType type, BufferUsage usage) :
	_elementCount(0),
	_elementSize(0),
	_handle(0)
{
	// TODO: implement
}

IBuffer::~IBuffer() {
	// TODO: implement
}

void IBuffer::LoadData(const void* data, size_t elementSize, size_t elementCount) {
	// TODO: implement
}

void IBuffer::Bind() {
	glBindBuffer((GLenum)_type, _handle);
}

void IBuffer::UnBind(BufferType type) {
	glBindBuffer((GLenum)type, 0);
}
