#include <assert.h> 			/*  matrix.c - 행렬연산에 관한 함수들 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <conio.h>
#include <FLOAT.H>
//#include "matrix.c"                   /*  행렬 연산에 관한 함수들 */
#define STRING 20                  /*  클래스 이름 한계 */
const double INF=DBL_MAX;                 /*  무한대 */

struct head_pattern* input_data(void); /*  입력화일로부터 학습패턴을 입력받는 함수 */
void cal_means(struct head_pattern *); /* 클래스별 학습패턴들의 평균을 구하는 함수 */
void cal_covariance(struct head_pattern *); /* 클래스별 학습패턴들의 공분산을 구한다 */
void inverse_covar(struct head_pattern *); /* 클래스별 공분산의 역행렬을 구한다 */
void log_det__covar(struct head_pattern *); /* 클래스별 공분산의 행렬식과 Log값 계산 */
struct head_pattern *decision_func(struct head_pattern *,double *); /* 미지패턴의 식별함수 */
void classify(struct head_pattern *); /* 미지패턴을 입력하여 해당 클래스를 결정하여 출력 */
int nfeature;                         /* 패턴의 차원수 */
int nclass;                           /* 클래스 종류 */
double DeterminantM(int dimension,double cov[]); /* 행렬식을 구하는 함수 */
double *BM(int dimension,const double b[],const double m[],double *result);
double *MinusB(int dimension,const double *m1,const double *m2,double *result);
void IM(int dimension, double *m, double *result);
double BB(int dimension, const double m1[], const double m2[]);


struct pattern{                       /* 한 개의 패턴을 위한 구조체 */
	double *vt;   
	struct pattern *next;
};
struct head_pattern{                   /* linked list로 된 패턴들의 해드 */
	char name[STRING];        /* 클래스의 이름 */
	int number;                 /* 패턴들의 갯수 */
	double *mean;              /* 패턴들의 평균 */
	double *cov;                /* 패턴들의 공분산 */
	double *icov;               /* 공분산의 역행렬 */
    double log_det_cov;          /* 공분산 행렬식의 대수 */
	struct pattern *pt;
	struct head_pattern *next;
};

void main()
{
 int i;
 struct head_pattern *hpt;// Head Pointer
 hpt=input_data(); //head pattern 포인터 반환해서 hpt 에 저장
 cal_means(hpt);
 cal_covariance(hpt);
 inverse_covar(hpt);
 log_det__covar(hpt);
 do{
	classify(hpt);
	do{
		printf("\nDo you again ? ");
	}while((i=getche())!='N' && i!='n' && i!='Y' && i!='y');
 }while(i=='Y' || i=='y');
}

struct head_pattern* input_data() // 첫 번째로 실행 : 데이터 입력
{
 struct head_pattern *bpt,*hpt=NULL;//hpt HeadPointer,bpt base pointer
 struct pattern *pt;
 double v;
 char s[STRING];
 int i,j,k,t=0;
 FILE *in;
 if((in=fopen("train.pat","rt"))==NULL){
	printf("Cannot open file \"training.pat\".");
	exit(1);
 }
 printf("input data 함수 실행\n");
 fscanf(in,"%d%d",&nclass,&nfeature);
 for(i=0;i<nclass;i++){
	bpt=(struct head_pattern *)malloc(sizeof(struct head_pattern));
	memset(bpt,NULL,sizeof(struct head_pattern));
	bpt->next=hpt;
	hpt=bpt;
	if( t ) strcpy(hpt->name,s);
	else{
          	fscanf(in,"%s",hpt->name);
		t=1;
	}
	pt=NULL;
	for(k=0;(fscanf(in,"%s",s) > 0) && sscanf(s,"%lf",&v) > 0;k++){ //스트링 들어오면 실패->새로운 클래스
      	hpt->pt=(struct pattern *)malloc(sizeof(struct pattern));
		memset(hpt->pt,NULL,sizeof(struct pattern));
		hpt->pt->next=pt;
		pt=hpt->pt;
		pt->vt=(double *)malloc(sizeof(double)*nfeature);
		memset(pt->vt,NULL,sizeof(double)*nfeature);
		*(pt->vt)=v;
		for(j=1;j<nfeature;j++)
			fscanf(in,"%lf",pt->vt+j);
	}
	hpt->number=k;
 }
 printf("\ninput OK\n");
 printf("number of class : %d\nnumber of feature : %d\n",nclass,nfeature);
 bpt=hpt;
 for(i=0;i<nclass;i++){
	printf("class %s :%d\n",bpt->name,bpt->number);
	bpt=bpt->next;
 }
 fclose(in);
 return hpt;
}

void cal_means(struct head_pattern *hpt) // 두번째로 실행 : 평균 계산
{
 struct pattern *pt;
 int i;
 while(hpt!=NULL){
	pt=hpt->pt;
    hpt->mean=(double*)malloc(sizeof(double)*nfeature);
	memset(hpt->mean,NULL,sizeof(double)*nfeature);
    while(pt!=NULL){
      	for(i=0;i<nfeature;i++)
			hpt->mean[i]+=pt->vt[i];
		pt=pt->next;
	}
	for(i=0;i<nfeature;i++) hpt->mean[i]/=hpt->number;
	hpt=hpt->next;
 }
}

void cal_covariance(struct head_pattern *hpt) // 세번째로 실행 : 분산 계산
{
 struct pattern *pt;
 int i,j;
 while(hpt!=NULL){
        pt=hpt->pt;
        hpt->cov=(double*)malloc(sizeof(double)*nfeature*nfeature);
		memset(hpt->cov,NULL,sizeof(double)*nfeature*nfeature);
        while(pt!=NULL){
        	     for(i=0;i<nfeature;i++){
               	for(j=0;j<nfeature;j++){
                    	hpt->cov[j*nfeature+i] +=
		 	     (hpt->mean[i] - pt->vt[i])*(hpt->mean[j] - pt->vt[j]);
	      	}
               }
               pt=pt->next;
        }
        for(i=0;i<nfeature;i++)
        	     for(j=0;j<nfeature;j++) hpt->cov[j*nfeature+i] /= hpt->number;
        hpt=hpt->next;
 }

}

void inverse_covar(struct head_pattern *hpt) // 네 번째 실행 : 역행렬 계산 
{
 while(hpt != NULL){
       // void IM(int dimension, double *m, double *result) /* 역행렬을 구하는 함수 */
	 hpt->icov=(double *)malloc(sizeof(double)*nfeature*nfeature);
	 memset(hpt->icov,NULL,sizeof(double)*nfeature*nfeature);
	IM(nfeature, hpt->cov,hpt->icov );

	/*  hpt->icov에 메모리 할당후, hpt->cov의 역행렬을 계산하여 대입  */
	hpt=hpt->next;
 }
}

void log_det__covar(struct head_pattern *hpt)
{
 while(hpt != NULL){
	hpt->log_det_cov = log(fabs(DeterminantM(nfeature,hpt->cov)));
          hpt=hpt->next;                           
 }
}

struct head_pattern *decision_func(struct head_pattern *hpt,double *pt)
{
 struct head_pattern *cp;
 double t,min = INF;
 double *temp1 = (double *)malloc(sizeof(double)*nfeature);
 memset(temp1,NULL,sizeof(double)*nfeature);
 double *temp2 = (double *)malloc(sizeof(double)*nfeature);
 memset(temp2,NULL,sizeof(double)*nfeature);
 while(hpt!=NULL){
     MinusB(nfeature,pt,hpt->mean,temp1); /* temp1 = pt - hpt->mean */
     BM(nfeature,temp1,hpt->icov,temp2); /* temp2 = temp1 * hpt->icov */
     if((t=(BB(nfeature,temp2,temp1) + hpt->log_det_cov)) < min ){ /* BB=temp2 * temp1 */
	/* 가 최소인 클래스로 미지패턴을 판별한다.  */
	  min = t;
	  cp = hpt;
     }
     hpt = hpt->next;
 }
 free(temp2);
 free(temp1);
 return cp;
}

void classify(struct head_pattern *hpt) // 클래스로 분류
{
 FILE *in;
 char fn[13];
 int i;
 double *pt = (double *)malloc(sizeof(double)*nfeature);
 memset(pt,NULL,sizeof(double)*nfeature);
 printf("\nInput data file name : ");
 scanf("%s",fn);
 if((in=fopen(fn,"rt"))==NULL){
	printf("\nCannot open file.\n");
	exit(1);
 }
 while(fscanf(in,"%lf",pt)==1){
	printf("%.1lf ",*pt);
	for(i=1; i<nfeature; i++){
		fscanf(in,"%lf",&pt[i]);
		printf("%.1lf ",pt[i]);
	}
	printf("class : %s\n",decision_func(hpt,pt)->name);
 }
 free(pt);
 fclose(in);
}

//#include <assert.h> 			/*  matrix.c - 행렬연산에 관한 함수들 */
/* 1행 행렬과 정방행렬과의 곱 */
double *BM(int dimension,const double b[],const double m[],double *result)
{
 int i,j;
 assert(b!=result);
 for(i=0;i<dimension;i++){
	result[i]=0;
	for(j=0;j<dimension;j++)
		result[i]+=b[j]*m[j*dimension+i];
 }
 return result;
}

/* 1행 행렬과 1열 행렬과의 곱 */
double BB(int dimension, const double m1[], const double m2[])
{
 double result;
 int i;
 result=0;
 for(i=0;i<dimension;i++)
		result+=m1[i]*m2[i];
 return result;
}

/* 1행 또는 1열의 행렬의 차 */
double *MinusB(int dimension,const double *m1,const double *m2,double *result)
{
 int i;
 for(i=0; i<dimension; i++)
	result[i]=m1[i]-m2[i];
 return result;
}

#define TINY 1.0e-100
void ludcmp(double *a,int n,int *indx,double *d)
{
     int i,imax,j,k;
     double big,dum,sum,temp,*vv;

     vv=(double *)malloc(sizeof(double)*n);
	 memset(vv,NULL,sizeof(double)*n);
     *d=1.0;
     for (i=0;i<n;i++) {
	big=0.0;
	for (j=0;j<n;j++)
	     if ((temp=fabs(a[i*n+j])) > big) big=temp;
	     if (big == 0.0) printf("Singular matrix in routine ludcmp");
	     vv[i]=1.0/big;
     }
     for (j=0;j<n;j++) {
		for (i=0;i<j;i++) {
			sum=a[i*n+j];
			for (k=0;k<i;k++) sum -= a[i*n+k]*a[k*n+j];
			a[i*n+j]=sum;
		}
		big=0.0;
		for (i=j;i<n;i++) {
			sum=a[i*n+j];
			for (k=0;k<j;k++)
				sum -= a[i*n+k]*a[k*n+j];
			a[i*n+j]=sum;
			if ( (dum=vv[i]*fabs(sum)) >= big) {
				big=dum;
				imax=i;
			}
		}
		if (j != imax) {
			for (k=0;k<n;k++) {
				dum=a[imax*n+k];
				a[imax*n+k]=a[j*n+k];
				a[j*n+k]=dum;
			}
			*d = -(*d);
			vv[imax]=vv[j];
		}
		indx[j]=imax;
		if (a[j*n+j] == 0.0) a[j*n+j]=TINY;
		if (j != n-1) {
			dum=1.0/(a[j*n+j]);
			for (i=j+1;i<n;i++) a[i*n+j] *= dum;
		}
	}
	free(vv);
}

#undef TINY
void lubksb(double *a,int n,int indx[],double b[])
{
	int i,ii=-1,ip,j;
	double sum;
	for (i=0;i<n;i++) {
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if (ii != -1) for (j=ii;j<i;j++) sum -= a[i*n+j]*b[j];
		else if (sum) ii=i;
		b[i]=sum;
	}
	for (i=n-1;i>=0;i--) {
		sum=b[i];
		for (j=i+1;j<n;j++) sum -= a[i*n+j]*b[j];
		if(a[i*n+i]==0) { printf("error :  / 0 ");exit(0); }
		b[i]=sum/a[i*n+i];
	}
}

void IM(int dimension, double *m, double *result) /* 역행렬을 구하는 함수 */
{
 double d,*col;
 int i,j,*indx;
 col=(double *)malloc(sizeof(double)*dimension);
 memset(col,NULL,sizeof(double)*dimension);
 indx=(int *)malloc(sizeof(int)*dimension);
 memset(indx, NULL,sizeof(int)*dimension);

 ludcmp(m,dimension,indx,&d);
 for(j=0;j<dimension;j++){
	for(i=0;i<dimension;i++) col[i]=0.0;
	col[j]=1.0;
	lubksb(m,dimension,indx,col);
	for(i=0;i<dimension;i++) result[i*dimension+j]=col[i];
 }
 free(col);
 free(indx);
}

double DeterminantM(int dimension,double cov[]) /* 행렬식을 구하는 함수 */
{
 double *m,det = 0.;
 int *indx,i;
 m = (double *)malloc(sizeof(double)*dimension*dimension);
 memset(m, NULL, sizeof(double)*dimension*dimension);
 indx = (int *)malloc(sizeof(int)*dimension);
 memset(indx, NULL, sizeof(int)*dimension);
 memcpy(m,cov,sizeof(double)*dimension*dimension);
 ludcmp(m,dimension,indx,&det);
 for(i=0; i<dimension; i++) det*= m[i*dimension + i];
 free(indx);
 free(m);
 return det;
}
