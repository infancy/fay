#include"fay.h"
#include"ray/raytracing.h"

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);
	//--stderrthreshold=0 --logtostderr=true
	//FLAGS_logtostderr = true;
	//FLAGS_stderrthreshold = 0;
	//FLAGS_v = 2;

	//view_model();
	//paricles();

	ray_cast();
	//light_gimza();
	return 0;
}