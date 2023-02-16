#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "hashtable.h"

typedef char i8;
typedef unsigned char u8;
typedef unsigned int u32;
#define NORTH 1
#define SOUTH 2
#define EAST 3
#define WEST 4
#define tab_depth 7

u32 W=8,H=8;
u32 SIZE;

#define MAX(x,y)(((x)>(y))?(x):(y))
#define MIN(x,y)(((x)<(y))?(x):(y))
//i8 xmin=99,ymin=99,xmax=0,ymax=0;
i8 cords[32][2]={0};
typedef struct{
    i8 xmin,ymin,xmax,ymax;

}bb;
hash_table* previous_states;

void printb(i8* b){
    for (int i=0;i<SIZE;++i) {
        if(i%W==0)
            printf("\n");
        if(b[i]==-2)
            printf("\033[31m @\033[0m");
        else
            printf("%2i",b[i]);
    }
    printf("\n");
}
bool testb(u32 avl,i8* b_o,bb bounds,u32 offset,int dir,int depth){
    i8 b[SIZE];
    memcpy(b, b_o, SIZE);
    i8 yoff=0,xoff=0;
    //direction of expansion
    if(dir == NORTH)yoff=-1;
    if(dir == SOUTH)yoff=1;
    if(dir == EAST)xoff=1;
    if(dir == WEST)xoff=-1;
    if(dir){
        //expand block
        i8 x=cords[offset][0],y=cords[offset][1];
        i8 count=b[y*W+x];
        b[y*W+x]=-2;
        //expand in direction
        while(count){
            x=(i8)(x+xoff);y=(i8)(y+yoff);
            //check bounds, using bounding box is slower in practice
            if(y<=0||y>=H-1||x<=0||x>=W-1)
                break;
            //found goal
            if(b[y*W+x] == -1){
                b[y*W+x]=-2;
                printb(b);
                return true;
            }
            //place block
            if(b[y*W+x] == 0){
                b[y*W+x]=-2;
                count--;
            }
        }
    }
    //check if we have already checked this exact state
    //8 seems to be the best for performance
    //~100x speedup from this alone
    if(depth<=tab_depth){
        if(upsert(previous_states,(u8*)b)){
            return false;
        }
    }
    //recalculate the bounding box to reduce the search space
    bounds=(bb){.ymax=0,.xmax=0,.xmin=(i8)W,.ymin=(i8)H};
    for(u32 mask=1,off=0;avl>=mask;mask<<=1,off+=1) {
        if(mask&avl){
            bounds.ymin=MIN(bounds.ymin,cords[off][1]);
            bounds.xmin=MIN(bounds.xmin,cords[off][0]);
            bounds.ymax=MAX(bounds.ymax,cords[off][1]);
            bounds.xmax=MAX(bounds.xmax,cords[off][0]);
        }
    }
    bounds.ymin=MIN(bounds.ymin, cords[31][1]-1);
    bounds.xmin=MIN(bounds.xmin, cords[31][0]-1);
    bounds.ymax=MAX(bounds.ymax, cords[31][1]+1);
    bounds.xmax=MAX(bounds.xmax, cords[31][0]+1);


    //check each block that hasn't been expanded
    for(u32 mask=1,off=0;avl>=mask;mask<<=1,off+=1){
        if(avl&mask){
            i8 x=cords[off][0],y=cords[off][1];
            //test each direction only if not touching bounding box
            if(y>bounds.ymin+1&&x!=bounds.xmin&&x!=bounds.xmax){
                if(testb(avl&~mask, b, bounds,off, NORTH,depth+1)){
                    printb(b);
                    return true;
                }
            }
            if(y<bounds.ymax-1&&x!=bounds.xmin&&x!=bounds.xmax){
                if(testb(avl&~mask, b, bounds, off, SOUTH,depth+1)){
                    printb(b);
                    return true;
                }
            }
            if(x>bounds.xmin+1&&y!=bounds.ymin&&y!=bounds.ymax){
                if(testb(avl&~mask, b,bounds, off, WEST,depth+1)){
                    printb(b);
                    return true;
                }
            }
            if(x<bounds.xmax-1&&y!=bounds.ymin&&y!=bounds.ymax){
                if(testb(avl&~mask, b, bounds, off, EAST,depth+1)){
                    printb(b);
                    return true;
                }
            }
        }
    }
    return false;
}
int main(int argc, const char* argv[]){
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    //load input file
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    fscanf(fp, "%u %u", &W, &H);
    SIZE=W*H;

    i8 board[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        int temp;
        fscanf(fp, "%d", &temp);
        board[i]=(i8)temp;
    }
    fclose(fp);

    u32 avl=0;
    i8 offset=0;
    bb bounds={.ymax=0,.xmax=0,.xmin=(i8)W,.ymin=(i8)H};
    for(u32 i=0;i<SIZE;i++){
        //keep track of outer bounds
        //updating continuously seems to be slower in practice
        if(board[i]>0){
            bounds.ymin=MIN(bounds.ymin, i / W);
            bounds.xmin=MIN(bounds.xmin, i % W);
            bounds.ymax=MAX(bounds.ymax, i / W);
            bounds.xmax=MAX(bounds.xmax, i % W);
            //keep track of blocks that can be expanded
            cords[offset][0]=(i8)(i%W);cords[offset][1]=(i8)(i/W);
            avl|=1<<offset++;
        }
        //expand bounding box past goal so that it doesn't get optimized out
        if(board[i]==-1){
            bounds.ymin=MIN(bounds.ymin, i/W-1);
            bounds.xmin=MIN(bounds.xmin, i%W-1);
            bounds.ymax=MAX(bounds.ymax, i/W+1);
            bounds.xmax=MAX(bounds.xmax, i%W+1);
            cords[31][0]=(i8)(i%W);cords[31][1]=(i8)(i/W);
        }
    }
    i8 g_x=cords[31][0],g_y=cords[31][1];
    //find blocks on same axis as goal and try them last
    //ranges from mildly faster to near instant
    for (u32 i = 0,ix=offset-1; i<offset; ++i){
        if(cords[i][0]==g_x||cords[i][1]==g_y){
            i8 tx=cords[i][0],ty=cords[i][1];
            cords[i][0]=cords[ix][0];cords[i][1]=cords[ix][1];
            cords[ix][0]=tx;cords[ix][1]=ty;
            ix--;
        }
    }
    //table of past states to avoid checking the same thing twice
    //more memory lets us same more states up to a point but takes more time to allocate when used
    //\frac{n!}{k!\left(n-k\right)!}\cdot\frac{4^{k}}{n}
    //current estimate (n-1)!/(k!(n-k)!) * 4^k
    long mem=(tgamma(offset)/((tgamma(tab_depth+1)*tgamma(offset+1-tab_depth)))*pow(4,tab_depth));
    printf("allocating table of %ld blocks",mem);
    previous_states= hash_table_init(mem,SIZE);

    printb(board);
    printf("%i blocks\n",offset);
    //todo multithreading
    bool solved=testb(avl, board, bounds, 0, 0, 0);
    printf("%s\n",solved?"solved":"no solution");
    printf("table used %d of %d slots for %d blocks",previous_states->used,previous_states->size,offset);
    return 0;
}
//8 5520
//9 7 800
//9 7 3324
//10 7 112708
//11 7 159470
//13 7 318739
//13 8 704365
//13 9 1205836
//13 10 1648988
//13 8 704365