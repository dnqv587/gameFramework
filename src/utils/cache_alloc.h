#pragma once
//内存池模块

#ifdef __cplusplus
extern "C" {
#endif
	//创建内存池
	struct cache_allocer* create_cache_allocer(int capacity, int elem_size);
	//销毁内存池
	void destroy_cache_allocer(struct cache_allocer* allocer);
	//分配内存
	void* cache_alloc(struct cache_allocer* allocer, int elem_size);
	//归还内存
	void cache_free(struct cache_allocer* allocer, void* mem);

#ifdef __cplusplus
}
#endif


