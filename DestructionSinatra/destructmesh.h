#ifndef DESTRUCTMESH_H
#define DESTRUCTMESH_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef MESHSTRUCT
#define MESHSTRUCT

typedef struct point{
    float x;
    float y;
    float z;
}Point;

typedef struct cube{
    float x,y,z;
    float sizeX,sizeY,sizeZ;
    float dirX,dirY,dirZ;
    Point center;
}Cube;

typedef struct mesh{
    Point* tab;
    int** links;
    int nbp;
    int nbl;
    Cube cube;
    int** neighbours;
    int* nbNeighbours;
}Mesh;

#endif

Point PointCreator(float x,float y,float z){
    Point newP;
    newP.x=x;
    newP.y=y;
    newP.z=z;
    return newP;
}

Point cpyPoint(Point p){
    return PointCreator(p.x,p.y,p.z);
}

Cube CubeCreator(float x,float y,float z,float sizeX,float sizeY,float sizeZ){
    Cube newC;
    newC.x=x;
    newC.y=y;
    newC.z=z;
    newC.sizeX=sizeX;
    newC.sizeY=sizeY;
    newC.sizeZ=sizeZ;
    newC.dirX=0;
    newC.dirY=0;
    newC.dirZ=0;
    newC.center.x = x+sizeX/2;
    newC.center.y = y+sizeY/2;
    newC.center.z = z+sizeZ/2;
    return newC;
}

int* LinkCreator(int a,int b){
    int* link = (int*) malloc(sizeof(int)*2);
    link[0]=a;
    link[1]=b;
    return link;
}

Mesh* allocMesh(){
    Mesh* mush = (Mesh*) malloc(sizeof(Mesh));
    mush->tab=NULL;
    mush->links=NULL;
    mush->nbNeighbours=NULL;
    mush->neighbours=NULL;
    return mush;
}

void giveStrenght(Mesh* mush,float dirX,float dirY,float dirZ){
    mush->cube.dirX+=dirX;
    mush->cube.dirY+=dirY;
    mush->cube.dirZ+=dirZ;
}

void translateMesh(Mesh* mush,float x,float y,float z){
    mush->cube.x+=x;
    mush->cube.y+=y;
    mush->cube.z+=z;
    mush->cube.center.x+=x;
    mush->cube.center.y+=y;
    mush->cube.center.z+=z;
}

void applyForce(Mesh* mush){
    translateMesh(mush,mush->cube.dirX,mush->cube.dirY,mush->cube.dirZ);
    mush->cube.dirX*=0.9;
    mush->cube.dirY*=0.9;
    mush->cube.dirZ*=0.9;
    if(mush->cube.dirX < 0.001) mush->cube.dirX=0;
    if(mush->cube.dirY < 0.001) mush->cube.dirY=0;
    if(mush->cube.dirZ < 0.001) mush->cube.dirZ=0;
}

void freeMesh(Mesh* mush){
    free(mush->tab);
    free(mush->links);
    if(mush->neighbours!=NULL){
        free(mush->nbNeighbours);
        free(mush->neighbours);
    }
    free(mush);
}

void freeMeshComplete(Mesh* mush){
    free(mush->tab);
    for(int i=0;i<mush->nbl;i++){
        free(mush->links[i]);
    }
    free(mush->links);
    if(mush->neighbours!=NULL){
        free(mush->nbNeighbours);
        free(mush->neighbours);
    }
    free(mush);
}

Mesh* fromCubeToTrueMesh(Cube cube,int nbP){
    Mesh* mush = allocMesh();
    mush->nbp = pow(nbP,3);
    int precalc = pow(nbP,2);
    mush->tab = (Point*) malloc(sizeof(Point)*(mush->nbp));
    mush->nbl=3*(mush->nbp-precalc);
    mush->links = (int**) malloc(sizeof(int*)*mush->nbl);
    //////////////////// ALLOUER ARRETES
    for(int i=0,comptP=0,comptL=0;i<nbP;i++){
        for(int j=0;j<nbP;j++){
            for(int k=0;k<nbP;k++){
                if(k!=0) mush->links[comptL++] = LinkCreator(comptP-1,comptP);
                if(j!=0) mush->links[comptL++] = LinkCreator(comptP-nbP,comptP);
                if(i!=0) mush->links[comptL++] = LinkCreator(comptP-precalc,comptP);
                //mush->tab[comptP++] = PointCreator(cube.x+(cube.sizeX*k)/nbP,cube.y+(cube.sizeY*j)/nbP,cube.z+(cube.sizeZ*i)/nbP);
                mush->tab[comptP++] = PointCreator(((float)k)/nbP,((float)j)/nbP,((float)i)/nbP);
            }
        }
    }
    mush->cube=cube;
    return mush;
}

int countLinks(Mesh* mush){
    int count=0;
    for(int i=0;i<mush->nbl;i++){
        if(mush->links[i][0]!=-1 && mush->links[i][1]!=-1) count++;
    }
    return count;
}

int cleanLinks(Mesh* mush){
    int newNbl=countLinks(mush);
    int oldNbl=mush->nbl;
    int** oldLink = mush->links;
    mush->links = (int**) malloc(sizeof(int*)*newNbl);
    for(int i=0,count=0;i<mush->nbl;i++){
        if(oldLink[i][0]!=-1 && oldLink[i][1]!=-1) mush->links[count++] = LinkCreator(oldLink[i][0],oldLink[i][1]);
    }
    for(int i=0;i<mush->nbl;i++){
        free(oldLink[i]);
    }
    free(oldLink);
    mush->nbl=newNbl;
    return oldNbl-newNbl;
}

void calcNeighbours(Mesh* mush){
    int** neighbours = (int**) malloc(sizeof(int*)*mush->nbp);
    int* nbNeighbours = (int*) malloc(sizeof(int)*mush->nbp);
    for(int i=0;i<mush->nbp;i++){
        nbNeighbours[i]=0;
        neighbours[i]=(int*) malloc(sizeof(int)*8);
    }
    for(int i=0;i<mush->nbl;i++){
        neighbours[mush->links[i][0]][nbNeighbours[mush->links[i][0]]++] = mush->links[i][1];
        neighbours[mush->links[i][1]][nbNeighbours[mush->links[i][1]]++] = mush->links[i][0];
    }

    if(mush->nbNeighbours!=NULL){
        for(int i=0;i<mush->nbp;i++){
            free(mush->neighbours[i]);
        }
        free(mush->neighbours);
        free(mush->nbNeighbours);
    }
    mush->neighbours=neighbours;
    mush->nbNeighbours=nbNeighbours;
}

void labelRec(Mesh* mush,int i,int* labels,int lab){
    if(labels[i]!=-1) return;
    labels[i]=lab;
    for(int j=0;j<mush->nbNeighbours[i];j++){
        labelRec(mush,mush->neighbours[i][j],labels,lab);
    }
}

int countLabelinLinks(Mesh* mush,int* labels,int num){
    int count=0;
    for(int i=0;i<mush->nbl;i++){
        if((labels[mush->links[i][0]]==num) && (count++)) {;}
    }
    return count;
}

int countLabel(int* labels,int size,int num){
    int count=0;
    for(int i=0;i<size;i++){
        if((labels[i]==num) && (count++)) {;}
    }
    return count;
}

void calcEnvelop(Mesh* mush){
    float xmin,xmax,ymin,ymax,zmin,zmax;
    xmin = mush->tab[0].x;
    xmax = mush->tab[0].x;
    ymin = mush->tab[0].y;
    ymax = mush->tab[0].y;
    zmin = mush->tab[0].z;
    zmax = mush->tab[0].z;
    //fprintf(stderr,"%d : %f %f %f %f %f %f\n",mush->nbp,xmin,xmax,ymin,ymax,zmin,zmax);
    for(int i=0;i<mush->nbp;i++){
        if(xmin>mush->tab[i].x) xmin=mush->tab[i].x;
        if(xmax<mush->tab[i].x) xmax=mush->tab[i].x;
        if(ymin>mush->tab[i].y) ymin=mush->tab[i].y;
        if(ymax<mush->tab[i].y) ymax=mush->tab[i].y;
        if(zmin>mush->tab[i].z) zmin=mush->tab[i].z;
        if(zmax<mush->tab[i].z) zmax=mush->tab[i].z;
    }
    //fprintf(stderr,"%f %f %f %f %f %f\n",xmin,xmax,ymin,ymax,zmin,zmax);
    mush->cube = CubeCreator(xmin,ymin,zmin,xmax-xmin,ymax-ymin,zmax-zmin);
}

void calcEnvelopWOLD(Mesh* mush,Cube cube){
    float xmin,xmax,ymin,ymax,zmin,zmax;
    xmin = mush->tab[0].x;
    xmax = mush->tab[0].x;
    ymin = mush->tab[0].y;
    ymax = mush->tab[0].y;
    zmin = mush->tab[0].z;
    zmax = mush->tab[0].z;
    //fprintf(stderr,"%d : %f %f %f %f %f %f\n",mush->nbp,xmin,xmax,ymin,ymax,zmin,zmax);
    for(int i=0;i<mush->nbp;i++){
        if(xmin>mush->tab[i].x) xmin=mush->tab[i].x;
        if(xmax<mush->tab[i].x) xmax=mush->tab[i].x;
        if(ymin>mush->tab[i].y) ymin=mush->tab[i].y;
        if(ymax<mush->tab[i].y) ymax=mush->tab[i].y;
        if(zmin>mush->tab[i].z) zmin=mush->tab[i].z;
        if(zmax<mush->tab[i].z) zmax=mush->tab[i].z;
    }
    //fprintf(stderr,"%f %f %f %f %f %f\n",xmin,xmax,ymin,ymax,zmin,zmax);
    mush->cube = CubeCreator(cube.x+cube.sizeX*xmin,cube.y+cube.sizeY*ymin,cube.z+cube.sizeZ*zmin,cube.sizeX*(xmax-xmin),cube.sizeY*(ymax-ymin),cube.sizeY*(zmax-zmin));
    if(xmin==xmax) mush->cube.sizeX=0;
    if(ymin==ymax) mush->cube.sizeY=0;
    if(zmin==zmax) mush->cube.sizeZ=0;
    for(int i=0;i<mush->nbp;i++){
        if(xmax-xmin!=0) mush->tab[i].x = (mush->tab[i].x-xmin)/(xmax-xmin);
        else mush->tab[i].x=0;
        if(xmax-xmin!=0) mush->tab[i].y = (mush->tab[i].y-ymin)/(ymax-ymin);
        else mush->tab[i].y=0;
        if(xmax-xmin!=0) mush->tab[i].z = (mush->tab[i].z-zmin)/(zmax-zmin);
        else mush->tab[i].z=0;
    }
    giveStrenght(mush,cube.dirX,cube.dirY,cube.dirZ);
}

int indi(int* tab,int n,int size){
    for(int i=0;i<size;i++){
        if(tab[i]==n) return i;
    }
    fprintf(stderr,"lol\n");
    return -1;
}

int separatePoint(Mesh* mush,Mesh*** meshTab){
    int* labels = (int*) malloc(sizeof(int)*mush->nbp);
    for(int i=0;i<mush->nbp;i++) labels[i]=-1;
    int lab=0;
    for(int i=0;i<mush->nbp;i++){
        if(labels[i]==-1) labelRec(mush,i,labels,lab++);
    }
    (*meshTab) = (Mesh**) malloc(sizeof(Mesh*)*lab);
    for(int i=0;i<lab;i++){
        //fprintf(stderr,"%d : %d points && %d links\n",i,countLabel(labels,mush->nbp,i),countLabelinLinks(mush,labels,i));
        (*meshTab)[i] = allocMesh();
        (*meshTab)[i]->nbp = countLabel(labels,mush->nbp,i);
        (*meshTab)[i]->tab = (Point*) malloc(sizeof(Point)*(*meshTab)[i]->nbp);
        (*meshTab)[i]->nbl = countLabelinLinks(mush,labels,i);
        (*meshTab)[i]->links = (int**) malloc(sizeof(int*)*(*meshTab)[i]->nbl);
        int* tab = (int*) malloc(sizeof(int)*(*meshTab)[i]->nbp);
        for(int j=0,count=0;j<mush->nbp;j++){
            if(labels[j]==i) {
                tab[count] = j;
                (*meshTab)[i]->tab[count++]=cpyPoint(mush->tab[j]);
            }
        }
        for(int j=0,count=0;j<mush->nbl;j++){
            if(labels[mush->links[j][0]]==i) (*meshTab)[i]->links[count++]=LinkCreator(indi(tab,mush->links[j][0],(*meshTab)[i]->nbp),indi(tab,mush->links[j][1],(*meshTab)[i]->nbp));
        }
        free(tab);
        //calcNeighbours((*meshTab)[i]);
        calcEnvelopWOLD((*meshTab)[i],mush->cube);
        float randx = (*meshTab)[i]->cube.x - mush->cube.x;
        float randy = (*meshTab)[i]->cube.y - mush->cube.y;
        float randz = (*meshTab)[i]->cube.z - mush->cube.z;
        /*float randx = 2.0 * qrand()/RAND_MAX -1.0;
        float randy = 2.0 * qrand()/RAND_MAX -1.0;
        float randz = 2.0 * qrand()/RAND_MAX -1.0;*/
        giveStrenght((*meshTab)[i],randx,randy,randz);
    }
    free(labels);
    return lab;
}

bool Collision(Cube a,Cube b, bool rec = true){
    if(a.x < b.x xor a.x+a.sizeX<b.x) return true;
    if(a.y < b.y xor a.y+a.sizeY<b.y) return true;
    if(a.z < b.z xor a.y+a.sizeZ<b.z) return true;
    if(rec) return Collision(b,a,false);
    return false;
}

Mesh** breakMesh(Mesh* mush,float x,float y,float z,int* nbmesh){
    for(int i=0;i<mush->nbl;i++){
        if((mush->tab[mush->links[i][0]].x<x) xor (mush->tab[mush->links[i][1]].x<x)){
            mush->links[i][0]=-1;
            mush->links[i][1]=-1;
        }
        if((mush->tab[mush->links[i][0]].y<y) xor (mush->tab[mush->links[i][1]].y<y)){
            mush->links[i][0]=-1;
            mush->links[i][1]=-1;
        }
        if((mush->tab[mush->links[i][0]].z<z) xor (mush->tab[mush->links[i][1]].z<z)){
            mush->links[i][0]=-1;
            mush->links[i][1]=-1;
        }
    }
    cleanLinks(mush);
    calcNeighbours(mush);
    Mesh** meshTab = NULL;
    *nbmesh = separatePoint(mush,&meshTab);
    freeMeshComplete(mush);
    return meshTab;
}


/*int main(int argc,char** argv){
    Cube cube = CubeCreator(0,0,10,2,2,2);
    Mesh* mush = fromCubeToTrueMesh(cube,10);
    int nbMesh;
    Mesh** meshTab = breakMesh(mush,0.5,0.5,0.5,&nbMesh);
    //fprintf(stderr,"%d == %d\n",countLinks(mush),mush->nbl);
    for(int i=0;i<nbMesh;i++){
        fprintf(stderr,"%d : %d point && %d links\n",i,meshTab[i]->nbp,meshTab[i]->nbl);
        //freeMeshComplete(meshTab[i]);
    }
    meshTab = breakMesh(meshTab[0],0.5,0.5,0.5,&nbMesh);
    for(int i=0;i<nbMesh;i++){
        fprintf(stderr,"%d : %d point && %d links\n",i,meshTab[i]->nbp,meshTab[i]->nbl);
        //freeMeshComplete(meshTab[i]);
    }
    //free(meshTab);
}*/

#endif // DESTRUCTMESH_H
