#include <stdio.h>
#include <stdbool.h>

typedef char i8;
typedef unsigned int u32;
#define N 1
#define S 2
#define E 3
#define W 4

typedef struct{
    i8 b[64] ;
}game;

#define MAX(x,y)(((x)>(y))?(x):(y))
#define MIN(x,y)(((x)<(y))?(x):(y))
i8 xmin=7,ymin=7,xmax=0,ymax=0;
i8 cords[32][2]={0};

game board={
        0,0,0,0,0,0,0,0,
        0,0,0,0,-1,0,0,0,
        2,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,2,
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,0,1,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
};
void printb(game* g){
    for (int i = 0; i < 64; ++i) {
        if(i%8==0)
            printf("\n");
        if(g->b[i]==-2)
            printf(" @");
        else
            printf("%2i",g->b[i]);
    }
    printf("\n");
}
bool testb(u32 avl,game g,u32 offset,int dir){
    i8 yoff=0,xoff=0;
    //direction of expansion
    if(dir==N)yoff=-1;
    if(dir==S)yoff=1;
    if(dir==E)xoff=1;
    if(dir==W)xoff=-1;
    if(dir){
        //expand block
        i8 x=cords[offset][0],y=cords[offset][1];
        i8 count=g.b[y*8+x];
        g.b[y*8+x]=-2;
        //expand in direction
        while(count){
            x=(i8)(x+xoff);y=(i8)(y+yoff);
            //check bounds, using bounding box is slower in practice
            if(y<0||y>7||x<0||x>7)
                break;
            //found goal
            if(g.b[y*8+x]==-1){
                g.b[y*8+x]=-2;
                printb(&g);
                return true;
            }
            //place block
            if(g.b[y*8+x]==0){
                g.b[y*8+x]=-2;
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
                if(testb(avl&~mask,g,off,N)){
                    printb(&g);
                    return true;
                }
            }
            if(y<ymax-1&&x!=xmin&&x!=xmax){
                if(testb(avl&~mask,g,off,S)){
                    printb(&g);
                    return true;
                }
            }
            if(x>xmin+1&&y!=ymin&&y!=ymax){
                if(testb(avl&~mask,g,off,W)){
                    printb(&g);
                    return true;
                }
            }
            if(x<xmax-1&&y!=ymin&&y!=ymax){
                if(testb(avl&~mask,g,off,E)){
                    printb(&g);
                    return true;
                }
            }
        }
    }
    return false;
}
int main() {
    u32 avl=0;
    i8 offset=0;
    for(i8 i=0;i<64;i++){
        //keep track of blocks that can be expanded
        if(board.b[i]>0){
            cords[offset][0]=(i8)(i%8);cords[offset][1]=(i8)(i/8);
            avl|=1<<offset++;
        }
        //keep track of outer bounds
        //updating continuously seems to be slower in practice
        if(board.b[i]>0||board.b[i]==-1){
            ymin=MIN(ymin,i/8);
            xmin=MIN(xmin,i%8);
            ymax=MAX(ymax,i/8);
            xmax=MAX(xmax,i%8);
        }
    }
    printb(&board);
    printf("%i blocks\n",offset);
    bool solved=testb(avl,board,0,0);
    printf("%s",solved?"solved":"no solution");
    return 0;
}
