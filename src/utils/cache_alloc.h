#pragma once
//�ڴ��ģ��

#ifdef __cplusplus
extern "C" {
#endif
	//�����ڴ��
	struct cache_allocer* create_cache_allocer(int capacity, int elem_size);
	//�����ڴ��
	void destroy_cache_allocer(struct cache_allocer* allocer);
	//�����ڴ�
	void* cache_alloc(struct cache_allocer* allocer, int elem_size);
	//�黹�ڴ�
	void cache_free(struct cache_allocer* allocer, void* mem);

#ifdef __cplusplus
}
#endif

