#include"stdafx.h"
#include<windows.h>
#include"cv.h"
#include"highgui.h"
#include"stdio.h"

void skipframe(CvCapture* capture,int n){
	for(int i=1;i<=n;i++)
	cvQueryFrame(capture);                                                                              //����Ƶ������һ֡����ȡ��һ֡
}

void cvSkinSegment(IplImage* img, IplImage* mask){
	CvSize imageSize = cvSize(img->width, img->height);
	IplImage *imgY = cvCreateImage(imageSize, IPL_DEPTH_8U, 1);
	IplImage *imgCr = cvCreateImage(imageSize, IPL_DEPTH_8U, 1);
	IplImage *imgCb = cvCreateImage(imageSize, IPL_DEPTH_8U, 1);
	IplImage *imgYCrCb = cvCreateImage(imageSize, img->depth, img->nChannels);
	cvCvtColor(img,imgYCrCb,CV_BGR2YCrCb);																//��BGRͼƬת����YCrCb��ʽͼƬ
	
	/*��Y��Cr��Cbͨ�����룬Ȼ��������ָ��ֱ��ÿһ��ͨ�������ص���д���*/
	cvSplit(imgYCrCb, imgY, imgCr, imgCb, 0);
	int y, cr, cb, l, x1, y1, value;
	unsigned char *pY, *pCr, *pCb, *pMask;
	pY = (unsigned char *)imgY->imageData;
	pCr = (unsigned char *)imgCr->imageData;
	pCb = (unsigned char *)imgCb->imageData;
	pMask = (unsigned char *)mask->imageData;
	
	cvSetZero(mask);																					 //��maskָ���ͼƬ���ص��ֵ������
	l = img->height * img->width;																		 //�����ܵ����ص���������ȷ�������ѭ������
	 
	/*��Cb Cr�ռ����ҵ�һ��������ϳ����ɫ�ֲ�����Բ�Σ�Ȼ�������Բ�������ڵ����ص���Ϊ��ɫ*/
	for (int i = 0; i < l; i++){
		y  = *pY;
		cr = *pCr;
		cb = *pCb;
		cb -= 109;
		cr -= 152;
		x1 = (819*cr-614*cb)/32 + 51;
		y1 = (819*cr+614*cb)/32 + 77;
		x1 = x1*41/1024;
		y1 = y1*73/1024;
		value = x1*x1+y1*y1;
		if(y<100)	(*pMask)=(value<700) ? 255:0;                                                        //����ֵ�жϵ�����
		else		(*pMask)=(value<850)? 255:0;
		pY++;
		pCr++;
		pCb++;
		pMask++;
	}
	cvReleaseImage(&imgY);
	cvReleaseImage(&imgCr);
	cvReleaseImage(&imgCb);
	cvReleaseImage(&imgYCrCb);
}
int main()
{
	/*ÿ�����г��򶼽���־�ļ���գ����㿴�˴εõ�������*/
	FILE *fp;
	fp=fopen("d:\\gesturebook.log","w");
	fclose(fp);    

	/*����һ��ʵ����յķ���
	remove("d:\\gesturebook.log");  
	*/

	//��������ϵͳʱ��ı���
	time_t timer;
	struct tm *ptrtime;

	cvNamedWindow( "origin",CV_WINDOW_AUTOSIZE);     //��������ΪORIGIN����
	CvCapture* capture;                  
	capture = cvCreateCameraCapture(0);              //������ͷ��ȡ��Ƶ
	IplImage* frame;                      
	skipframe(capture,3);                            //����Ƚ���Ҫ����Ϊ�˴����������жϵ�һ֡�õ�����ֵ����Ϊ�յ��³����ڽ�������break�����������������Ϊ���������ͷ��ȡ�йأ���Ϊ��Щ����ֱ���ú���cvQueryFrame�õ��ĵ�һ֡���Ƿǿյģ���ô�˴��Ϳ��Բ���
	while(1) {                                       //��һ��һֱΪ�������ʹ֮һֱ�ܴ�����ͷ��ȡ֡��ֱ���ֶ�����һ��esc�Ķ���ʹ֮����ѭ��
		frame = cvQueryFrame( capture );
		if( !frame ) break;              
		//cvShowImage( "origin", frame );	                                       //������������
		IplImage* dstcrcb=cvCreateImage(cvGetSize(frame),8,1);
		cvSkinSegment(frame,dstcrcb);                                              //���÷�ɫ��⺯�������õ�֡ͼƬ
		cvDilate(dstcrcb,dstcrcb,NULL,1);                                          //�������ʹ����Ŵ����ص��ֵ 
		cvSmooth(dstcrcb,dstcrcb,CV_GAUSSIAN,3,3,0,0);//3x3                        //���и�˹ƽ������

		{   /*���ø���Ȥ��roi����*/
			int width = 640;
			int height = 160;
			cvSetImageROI(frame,cvRect(0,320,width,height));

			/*������*/
			IplImage *dsw = cvCreateImage(cvGetSize(dstcrcb), 8, 1);  
			IplImage *dst = cvCreateImage(cvGetSize(dstcrcb), 8, 3);  
			CvMemStorage *storage = cvCreateMemStorage(0);  
			CvSeq *first_contour = NULL;  

			cvThreshold(dstcrcb, dsw, 130, 255, CV_THRESH_BINARY);                                                                   //�ԻҶ�ͼ�������ֵ�����õ���ֵͼ��
			cvFindContours(dsw, storage, &first_contour, sizeof(CvContour),CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);                //Ѱ������
			cvZero(dst);  
			int cnt = 0;  
			for(; first_contour != 0; first_contour = first_contour->h_next)  
			{  
				cnt++;  
				CvScalar color = CV_RGB(255, 255,255);  
				cvDrawContours(dst, first_contour, color, color, 0, 2, CV_FILLED, cvPoint(0, 0));  
				CvRect rect = cvBoundingRect(first_contour,0);
				if(rect.width>100&&rect.width<500)                                                                                  //��ʶ��ķ�ɫ������µ����ƣ���ֹ���Ƶ�Ƥ���ı������ŵõ�����Ԥ�ڵ�����                                                                   
				{
					cvRectangle(frame, cvPoint(rect.x, rect.y), cvPoint(rect.x + 100, rect.y+130),CV_RGB(255, 255, 255), 1, 8, 0);  //�����ο���ʼ����ʶ������������Ͻǵ㣬��СΪ100*130����������Ƕ�����鷢��ʶ�����ƽϺõĴ�С
					if((rect.x-frame->origin)>=0&&(rect.x-frame->origin)<=2)                                                        //����ʶ������ƾ���ͼ�����߽�ľ����Ƿ��㹻������������������Ϊ����һ�����󻬶�������������һ����Ӧ��page up����
					{	 /*����time()������ȡ��ǰʱ��*/
						timer=time(NULL);
						/*����localtime()��������õ�ϵͳʱ��ת��Ϊָ��struct tm��ָ��ָ��Ľṹ��*/
						ptrtime = localtime( &timer ) ;
						keybd_event(VK_LEFT,0,0,0);                                  //����ϵͳ�����¼�������һ�������������page down���Ķ���
						if((fp=fopen("d:\\gesturebook.log","a"))==NULL)              //���ļ�
						{
						     printf("can't open the file!\n");
							 exit(1);
						}
						else
						fprintf(fp,"%s	:Page Down\n\n",asctime( ptrtime));         //��ʱ���Լ�����д���ļ���
    					fclose(fp);                                                 //�ر��ļ�
						skipframe(capture,7);                                       //������֡Ϊ�˷�ֹ��ȡ��β���
					}

					if((frame->origin+frame->width-rect.x-rect.width)>=0&&(frame->origin+frame->width-rect.x-rect.width)<=2)           ////����ʶ������ƾ���ͼ��ı߽�ľ����Ƿ��㹻������������������Ϊ����һ�����һ���������������һ����Ӧ��page down����
					{  
						/*����time()������ȡ��ǰʱ��*/
						timer=time(NULL);
						/*����localtime()��������õ�ϵͳʱ��ת��Ϊָ��struct tm��ָ��ָ��Ľṹ��*/
						ptrtime = localtime( &timer ) ;
						keybd_event(VK_RIGHT,0,0,0);                                 //����ϵͳ�����¼�������һ�������������page down���Ķ���
						if((fp=fopen("d:\\gesturebook.log","a"))==NULL)              //���ļ�
						{
						     printf("can't open the file!\n");
							 exit(1);
						}
						else
						fprintf(fp,"%s	:Page UP\n\n",asctime( ptrtime));
						fclose(fp);						
						skipframe(capture,5);
					}
				}
			} 
			cvResetImageROI(frame);//����ROI
			cvShowImage( "origin", frame );	
			//cvShowImage("out", dstcrcb);
			cvReleaseImage(&dst);
			cvReleaseImage(&dsw);
			cvReleaseMemStorage(&storage);  
		}

		char c = cvWaitKey(5);
		if( c == 27 ) break;
		cvReleaseImage(&dstcrcb);

	}
	cvReleaseCapture( &capture );
	cvDestroyWindow( "origin" );
}
