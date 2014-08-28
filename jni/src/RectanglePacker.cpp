#include "RectanglePacker.h"
#include "PooledAllocator.h"
#include <stdlib.h>
#include <assert.h>

struct RectangleNode{
	short x,y,w,h;
	RectangleNode *up,*down,*left,*right;
};

static int RectangleCompare(const void* lp1,const void* lp2){
	RectanglePackerRectangle *a=*(RectanglePackerRectangle**)lp1,*b=*(RectanglePackerRectangle**)lp2;

	if(a->priority>b->priority) return -1;
	else if(a->priority<b->priority) return 1;

	if(a->h>b->h) return -1;
	else if(a->h<b->h) return 1;

	if(a<b) return -1;
	else if(a>b) return 1;

	return 0;
}

int RectanglePacker::pack(int width,int height){
	if(rectangles.empty()) return 0;

	std::vector<RectanglePackerRectangle*> rects;

	for(unsigned int i=0;i<rectangles.size();i++){
		if(rectangles[i].w>width || rectangles[i].h>height) return -1;

		rectangles[i].index=-1;
		rectangles[i].x=0;
		rectangles[i].y=0;

		rects.push_back(&(rectangles[i]));
	}

	qsort(&(rects[0]),rects.size(),sizeof(RectanglePackerRectangle*),RectangleCompare);

	int totalHeight=0;
	int index=0;

	for(;;){
#ifdef _DEBUG
		printf("Packing page %d...\n",index);
#endif
		int h=packOnePage(width,height,index,rects);
		if(h>=0){
			totalHeight+=h;
			break;
		}else{
			totalHeight+=height;
			index++;
		}
	}

#ifdef _DEBUG
	printf("Packing completed.\n",index);
#endif

	return totalHeight;
}

int RectanglePacker::packOnePage(int width,int height,int index,std::vector<RectanglePackerRectangle*>& rects){
	//We use an algorithm similar to
	//<http://www.codeproject.com/Articles/210979/Fast-optimizing-rectangle-packing-algorithm-for-bu>
	//We use a four-way-linked list, just like what DLX algorithm does, to store empty rectangles.
	PooledAllocator<RectangleNode> nodes;
	RectangleNode *header;
	int ret=0;

	//init
	{
		header=nodes.allocate();
		RectangleNode node1={0,-1,width,-1,NULL,nodes.allocate(),NULL,NULL};
		RectangleNode node2={0,0,width,height,header,NULL,NULL,NULL};
		*(header)=node1;
		*(node1.down)=node2;
	}

	//for each unpacked rectangle
	for(unsigned int i=0;i<rects.size();i++){
		if(rects[i]->index>=0) continue;

		short w=rects[i]->w; //w of unpacked rectangle
		short h=rects[i]->h; //h of unpacked rectangle

		//handle a trivial case, which will cause bugs
		if(w<=0 || h<=0){
			rects[i]->index=index;
			rects[i]->x=0;
			rects[i]->y=0;
			continue;
		}

		short x0,y0; //position of current top-left block
		short w0,h0; //accumlated width and height
		bool bFound=false;

		//STEP 1: from left to right, and from top to bottom, find a suitable position.
		RectangleNode *lpTop=header,*lpCurrent=NULL;
		while(lpTop){
			//debug
			assert(lpTop->y==-1 && lpTop->h==-1);

			//check if possible max width is enough
			if(width-lpTop->x<w) break;

			//get the first block of this column
			lpCurrent=lpTop->down;
			while(lpCurrent){
				//debug
				assert(lpCurrent->x==lpTop->x && lpCurrent->w==lpTop->w);

				//check if possible max height is enough
				if(height-lpCurrent->y<h) break;

				bFound=true; //if we can pack it to current position
				RectangleNode *lp2=lpCurrent,*lp3;
				x0=lp2->x;
				y0=lp2->y;
				w0=lp2->w;

				for(;;){
					lp3=lp2;
					h0=lp3->h;

					for(;;){
						//check if height is enough
						if(h0>=h) break;

						//debug
						assert(y0+h0==lp3->y+lp3->h);
						assert(lp3->down==NULL || (lp3->down->x==lp3->x && lp3->down->w==lp3->w && lp3->down->y>=y0+h0));

						//go down
						lp3=lp3->down;

						//check it it's nothing or skipped some occupied block
						if(lp3==NULL || lp3->y>y0+h0){
							bFound=false;
							break;
						}

						//add height of this block
						h0+=lp3->h;
					}

					//check if we can't put it or the width is enough
					if(!bFound || w0>=w) break;

					//debug
					assert(x0+w0==lp2->x+lp2->w);
					assert(lp2->right==NULL || (lp2->right->y==lp2->y && lp2->right->h==lp2->h && lp2->right->x==x0+w0));

					//go right
					lp2=lp2->right;

					//check it it's nothing
					if(lp2==NULL){
						bFound=false;
						break;
					}

					//add width of this block
					w0+=lp2->w;
				}

				//check if we can pack it
				if(bFound) break;

				//go down
				lpCurrent=lpCurrent->down;
			}

			//check if we found one
			if(bFound) break;

			//debug
			assert(lpTop->right==NULL || lpTop->right->x==lpTop->x+lpTop->w);

			//get next column header
			lpTop=lpTop->right;
		}

		//check if we can't pack it
		if(!bFound){
			ret=-1;
			continue;
		}

		//pack it
		rects[i]->index=index;
		rects[i]->x=x0;
		rects[i]->y=y0;
		if(ret>=0 && y0+h>ret) ret=y0+h;
		
		//STEP 2a: subdivide the grid, add new column
		w0=0;
		for(;;){
			//debug
			assert(lpTop!=NULL);

			w0+=lpTop->w;

			//width matched exactly, don't need to subdivide
			if(w0==w) break;

			//subdivide it
			if(w0>w){
				RectangleNode *lpPrev=NULL;
				while(lpTop){
					//create new block
					RectangleNode *lp3=nodes.allocate();

					lp3->x=x0+w;
					lp3->y=lpTop->y;
					lp3->w=w0-w;
					lp3->h=lpTop->h;
					lp3->up=lpPrev;
					lp3->down=NULL;
					lp3->left=lpTop;
					lp3->right=lpTop->right;

					//add it to linked list
					lpTop->w=x0+w-lpTop->x;
					if(lpTop->right) lpTop->right->left=lp3;
					lpTop->right=lp3;
					if(lpPrev) lpPrev->down=lp3;

					//go down
					lpPrev=lp3;
					lpTop=lpTop->down;
				}

				break;
			}

			//go right
			lpTop=lpTop->right;
		}

		//STEP 2b: subdivide the grid, add new row (partially)
		h0=0;
		lpTop=lpCurrent;
		for(;;){
			//debug
			assert(lpTop!=NULL);

			h0+=lpTop->h;

			//width matched exactly, don't need to subdivide
			if(h0==h) break;

			//subdivide it
			if(h0>h){
				//go to the leftmost block
				while(lpTop->left) lpTop=lpTop->left;

				RectangleNode *lpPrev=NULL;
				while(lpTop){
					//create new block
					RectangleNode *lp3=nodes.allocate();

					lp3->x=lpTop->x;
					lp3->y=y0+h;
					lp3->w=lpTop->w;
					lp3->h=h0-h;
					lp3->up=lpTop;
					lp3->down=lpTop->down;
					lp3->left=lpPrev;
					lp3->right=NULL;

					//add it to linked list
					lpTop->h=y0+h-lpTop->y;
					if(lpTop->down) lpTop->down->up=lp3;
					lpTop->down=lp3;
					if(lpPrev) lpPrev->right=lp3;

					//go right
					lpPrev=lp3;
					lpTop=lpTop->right;
				}

				break;
			}

			//go down
			lpTop=lpTop->down;
		}

		//STEP 3: remove occupied blocks
		w0=0;
		for(;;){
			lpTop=lpCurrent;

			//we move right first because this node will be deleted soon
			lpCurrent=lpCurrent->right;

			//add width
			w0+=lpTop->w;

			//debug
			assert(w0<=w);

			//get the block above this block
			RectangleNode *lp0=lpTop->up;

			//debug
			assert(lp0!=NULL);

			//remove a column
			h0=0;
			for(;;){
				//debug
				assert(lpTop!=NULL);

				//add height
				h0+=lpTop->h;

				//debug
				assert(h0<=h);

				//cut the linked list into two parts
				if(lpTop->left) lpTop->left->right=NULL;
				if(lpTop->right) lpTop->right->left=NULL;

				//go down and delete current block
				RectangleNode *lp1=lpTop->down;
				nodes.deallocate(lpTop);
				lpTop=lp1;

				if(h0>=h){
					lp0->down=lpTop;
					if(lpTop) lpTop->up=lp0;
					break;
				}
			}

			if(w0>=w) break;

			//debug
			assert(lpCurrent!=NULL);
		}
	}

	return ret;
}
