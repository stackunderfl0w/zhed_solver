#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef char i8;
typedef unsigned int u32;
#define NORTH 1
#define SOUTH 2
#define EAST 3
#define WEST 4

u32 W=8,H=8;
u32 SIZE;

#define MAX(x,y)(((x)>(y))?(x):(y))
#define MIN(x,y)(((x)<(y))?(x):(y))
i8 xmin=7,ymin=7,xmax=0,ymax=0;
i8 cords[32][2]={0};

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
bool testb(u32 avl,i8* b_o,u32 offset,int dir){
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
    //check each block that hasn't been expanded
    for(u32 mask=1,off=0;avl>=mask;mask<<=1,off+=1){
        if(avl&mask){
            i8 x=cords[off][0],y=cords[off][1];
            //test each direction only if not touching bounding box
            if(y>ymin+1&&x!=xmin&&x!=xmax){
                if(testb(avl&~mask, b, off, NORTH)){
                    printb(b);
                    return true;
                }
            }
            if(y<ymax-1&&x!=xmin&&x!=xmax){
                if(testb(avl&~mask, b, off, SOUTH)){
                    printb(b);
                    return true;
                }
            }
            if(x>xmin+1&&y!=ymin&&y!=ymax){
                if(testb(avl&~mask, b, off, WEST)){
                    printb(b);
                    return true;
                }
            }
            if(x<xmax-1&&y!=ymin&&y!=ymax){
                if(testb(avl&~mask, b, off, EAST)){
                    printb(b);
                    return true;
                }
            }
        }
    }
    return false;
}
int main(int argc, const char* argv[])  {
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
    for(u32 i=0;i<SIZE;i++){
        //keep track of outer bounds
        //updating continuously seems to be slower in practice
        if(board[i]>0){
            ymin=MIN(ymin,i/W);
            xmin=MIN(xmin,i%W);
            ymax=MAX(ymax,i/W);
            xmax=MAX(xmax,i%W);
            //keep track of blocks that can be expanded
            cords[offset][0]=(i8)(i%W);cords[offset][1]=(i8)(i/W);
            avl|=1<<offset++;
        }
        //expand bounding box past goal so that it doesn't get optimized out
        if(board[i]==-1){
            ymin=MIN(ymin,i/W-1);
            xmin=MIN(xmin,i%W-1);
            ymax=MAX(ymax,i/W+1);
            xmax=MAX(xmax,i%W+1);
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
    printb(board);
    printf("%i blocks\n",offset);
    //todo multithreading
    //todo hash tables of previous states
    bool solved=testb(avl,board,0,0);
    printf("%s",solved?"solved":"no solution");
    return 0;
}
