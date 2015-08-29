#pragma once

/** \file
*/

#include <stdlib.h>
#include <assert.h>

/** A simple memory pooled chunk allocator.
\note sizeof(T) should be >=sizeof(void*)
*/

template <class T>
class PooledAllocator{
public:
	PooledAllocator():chunks(NULL),firstUnused(NULL),m_size(0),m_capacity(0){
	}
	~PooledAllocator(){
		clear();
	}
	void clear(){
		while(chunks){
			Chunk *nextChunk=chunks->next;
			free(chunks);
			chunks=nextChunk;
		}

		firstUnused=NULL;
		m_size=0;
		m_capacity=0;
	}
	intptr_t size() const{
		return m_size;
	}
	intptr_t capacity() const{
		return m_capacity;
	}
	void reserve(intptr_t size){
		while(size>m_capacity){
			intptr_t i=0,i2=0;
			if(chunks){
				i=chunks->size;
				i2=i*2;
			}else{
				i=16;
				i2=1024;
			}
			if(size+i<m_capacity) i=m_capacity-size;
			if(i>i2) i=i2;
			addChunk(i);
		}
	}
	/** Allocate an item. */
	T* allocate(){
		if(firstUnused==NULL){
			reserve(m_capacity<16?16:m_capacity*2);
		}

		T *node=firstUnused;
		firstUnused=*(T**)node;

		m_size++;

		return node;
	}
	/** Free an item.
	\note It should be in current allocator, and shouldn't be freed twice, or undefined behavior occurs.
	*/
	void deallocate(T* node){
		if(node==NULL) return;

		*(T**)node=firstUnused;
		firstUnused=node;

		m_size--;
	}
protected:
	struct Chunk{
		Chunk* next;
		intptr_t size;
		T item[1];
	};
	Chunk* chunks;
	T* firstUnused;
	intptr_t m_size,m_capacity;

	//internal function
	Chunk* addChunk(intptr_t size){
		assert(sizeof(T)>=sizeof(T*));
		Chunk* newChunk=(Chunk*)malloc(sizeof(Chunk*)+sizeof(intptr_t)+sizeof(T)*size);

		newChunk->next=chunks;
		newChunk->size=size;

		for(intptr_t i=0;i<size-1;i++){
			*(T**)(newChunk->item+i)=newChunk->item+(i+1);
		}
		*(T**)(newChunk->item+(size-1))=firstUnused;
		firstUnused=newChunk->item;

		m_capacity+=size;

		chunks=newChunk;
		return newChunk;
	}
};
