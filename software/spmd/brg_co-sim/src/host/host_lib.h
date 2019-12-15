#ifndef VCS_OFFLOAD_HOST_LIB_H
#define VCS_OFFLOAD_HOST_LIB_H

int device_malloc(void** dev_ptr, size_t size);
int memcpy_host_to_device(const void* dev_dst, const void* src, size_t size);
int memcpy_device_to_host(const void* dev_src, void* dest, size_t size);
int device_go();
void device_stop();
int host_recv();


#endif
