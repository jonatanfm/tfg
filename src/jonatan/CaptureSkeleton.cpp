#include "CaptureSkeleton.h"

void CaptureSkeleton::trackSkeleton(const NUI_SKELETON_FRAME& frame){

	for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
        NUI_SKELETON_TRACKING_STATE state = frame.SkeletonData[i].eTrackingState;
        if (NUI_SKELETON_TRACKED == state) {
            
			tot++;
			saveBoneLength(frame.SkeletonData[i].SkeletonPositions[0],frame.SkeletonData[i].SkeletonPositions[1],0);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[1],frame.SkeletonData[i].SkeletonPositions[2],1);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[2],frame.SkeletonData[i].SkeletonPositions[3],2);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[2],frame.SkeletonData[i].SkeletonPositions[4],3);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[4],frame.SkeletonData[i].SkeletonPositions[5],4);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[5],frame.SkeletonData[i].SkeletonPositions[6],5);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[6],frame.SkeletonData[i].SkeletonPositions[7],6);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[2],frame.SkeletonData[i].SkeletonPositions[8],7);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[8],frame.SkeletonData[i].SkeletonPositions[9],8);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[9],frame.SkeletonData[i].SkeletonPositions[10],9);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[10],frame.SkeletonData[i].SkeletonPositions[11],10);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[0],frame.SkeletonData[i].SkeletonPositions[12],11);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[12],frame.SkeletonData[i].SkeletonPositions[13],12);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[13],frame.SkeletonData[i].SkeletonPositions[14],13);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[14],frame.SkeletonData[i].SkeletonPositions[15],14);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[0],frame.SkeletonData[i].SkeletonPositions[16],15);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[16],frame.SkeletonData[i].SkeletonPositions[17],16);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[17],frame.SkeletonData[i].SkeletonPositions[18],17);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[18],frame.SkeletonData[i].SkeletonPositions[19],18);
			
			//float a=getAngle(frame.SkeletonData[i].SkeletonPositions[5],frame.SkeletonData[i].SkeletonPositions[4],frame.SkeletonData[i].SkeletonPositions[6]);			
			//float b=getAngle(frame.SkeletonData[i].SkeletonPositions[9],frame.SkeletonData[i].SkeletonPositions[8],frame.SkeletonData[i].SkeletonPositions[10]);
		}
        else if (NUI_SKELETON_POSITION_ONLY == state) {

        }
    }

}

void CaptureSkeleton::saveBoneLength(const Vector4& skel1,const Vector4& skel2,int pos){

	Joint tmp = Joint(skel2.x-skel1.x,skel2.y-skel1.y,skel2.z-skel1.z);

	float res = abs(sqrt((tmp.x*tmp.x) + (tmp.y*tmp.y) +(tmp.z*tmp.z)));

	suma[pos]=suma[pos]+res;
	

    if(res>maxim[pos]) maxim[pos]=res;
    if(res<minim[pos]) minim[pos]=res;
	float tmp2;

	tmp2=aver[pos];
	
    aver[pos]=suma[pos]/tot;

	sumaCuadrada[pos]=sumaCuadrada[pos]+(res*res);
	
	
}

float CaptureSkeleton::getAngle(const Vector4& skel1,const Vector4& skel2,const Vector4& skel3){

	Joint skel12 = Joint(skel2.x-skel1.x,skel2.y-skel1.y,skel2.z-skel1.z);
	Joint skel13 = Joint(skel3.x-skel1.x,skel3.y-skel1.y,skel3.z-skel1.z);

	float topEquation, bottomEquation;

	topEquation = (skel12.x*skel13.x)+(skel12.y*skel13.y)+(skel12.z*skel13.z);
	bottomEquation = sqrt( (skel12.x*skel12.x)+(skel12.y*skel12.y)+(skel12.z*skel12.z) )*
					 sqrt( (skel13.x*skel13.x)+(skel13.y*skel13.y)+(skel13.z*skel13.z) );


	float result = acos (topEquation/bottomEquation) * 180.0 / PI;

	return result;
}