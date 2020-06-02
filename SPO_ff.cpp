#include <iostream>
#include <functional>
#include <random>
#include <time.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <pthread.h>

#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>

using namespace std;

class Particle
{
	public:
		float X, Y;
		float speedX, speedY;
};

class Local_Min
{
	public:
		float localX, localY;
		float localMin = 100000;
};

class Global_Min
{
    public:
		float globalX, globalY;
		float globalMin = 100000;
};

class Window
{
	public:
		float XStart = 0;
		float YStart = 0;
		float XEnd = 1000;
		float YEnd = 900;		
};

float F(float x, float y)
{
	//return 2*x + y;
    return x + y;
};

float RandomNumberGenerator(float seed, float min, float max) 
{
	thread_local static std::mt19937 rng(seed);
	thread_local std::uniform_real_distribution<float> urd;
	return urd(rng, decltype(urd)::param_type{min, max});
}

void init(float _seed, vector<Global_Min> &threadGlobalMin, int N, int NIteration, float a,float b,float c, int nw, Particle *p, Local_Min *local, Global_Min *global, Window *window)
{
	ff::ParallelFor pr(nw);
	cout.precision(4);
	
	Local_Min 	*thread_local_min = new Local_Min[nw];
	
	pr.parallel_for_idx(0L, N, 1, 0, [&](const long start, const long end, const long thid)
	{
		//calculating the index of the current thread
		int th_id = floor((float)start/(ceil((float)N/(float)nw)));
		
		//Giving starting point for the current particle
		for(int i=start; i<end; i++)
		{
			//cout<<"The initial points of Particles"<<endl;
			p[i].X = RandomNumberGenerator(_seed, window->XStart, window->XEnd);
			p[i].Y = RandomNumberGenerator(_seed, window->XStart, window->XEnd);		
			//cout << p[i].X << " " << p[i].Y << endl;
		
			//Giving initial speed for the current Particle
			p[i].speedX = RandomNumberGenerator(_seed, -10, 10);
			p[i].speedY = RandomNumberGenerator(_seed, -10, 10); 		
			//cout<<"Initial speeds of "<<i<<"th partical: "<<p[i].speedX<<" "<<p[i].speedY<<endl;
		}
		
		//Finding Local_Min of the thread
		for(int i=start; i<end; i++)
		{
			float func = F(p[i].X , p[i].Y);
		
			if(func < thread_local_min[th_id].localMin)
			{
				thread_local_min[th_id].localX = p[i].X;	
				thread_local_min[th_id].localY = p[i].Y;	
				thread_local_min[th_id].localMin = func;	
			}
		}
		
	});
	
	//-----------------------------Barrier---------------------------------//
	//Finding Local_Min and Global_Min
	for(int i=0; i<nw; i++)
	{		
		if(thread_local_min[i].localMin < local->localMin)
		{
			local->localX = thread_local_min[i].localX;	
			local->localY = thread_local_min[i].localY;	
			local->localMin = thread_local_min[i].localMin;	
		
			if(local->localMin < global->globalMin)
			{
				global->globalX	= local->localX;	
				global->globalY = local->localY;	
				global->globalMin = local->localMin;
			}
		}
	}
}

float iteration(float _seed, vector<Global_Min> &threadGlobalMin, int N, int NIteration, float a,float b,float c, int nw, Particle *p, Local_Min *local, Global_Min *global, Window *window)
{
	init(_seed, ref(threadGlobalMin), N, NIteration, a,b,c, nw, ref(p),ref(local),ref(global), ref(window));
	
	ff::ParallelFor pr(nw);
	
	Local_Min 	*thread_local_min = new Local_Min[nw];
	
	//Starting our iterations
	for(int it=0; it<NIteration; it++)
	{		
		pr.parallel_for_idx(0L, N, 1, 0, [&](const long start, const long end, const long thid)
		{	
			float velocity;
			
			//calculating the index of the current thread
			int th_id = floor((float)start/(ceil((float)N/(float)nw)));
			
			//Updating velocity
			//cout<<"The new velocity of Particles"<<endl
			for(int i=start; i<end; i++)
			{
				velocity  =	a * p[i].speedX + 
							b * RandomNumberGenerator(_seed, 0, 1) * (p[i].X - local->localX) +
							c * RandomNumberGenerator(_seed, 0, 1) * (p[i].X - global->globalX);
							
				p[i].speedX = (velocity > 10) ? 10 : ((velocity < -10) ? -10 : velocity);
				
				velocity  =	a * p[i].speedY + 
							b * RandomNumberGenerator(_seed, 0, 1) * (p[i].Y - local->localY) +
							c * RandomNumberGenerator(_seed, 0, 1) * (p[i].Y - global->globalY);
				
				
				p[i].speedY = (velocity > 10) ? 10 : ((velocity < -10) ? -10 : velocity);
				//cout<<p[i].speedX <<" "<<p[i].speedY<<endl; 
			}
			
			
			//Updating positions
			//cout<<"The new positions of the points of Particles"<<endl;
			for(int i=start; i<end; i++)
			{				
				p[i].X = p[i].X + p[i].speedX;
				p[i].Y = p[i].Y + p[i].speedY;
							
				if(p[i].X < window->XStart)
					p[i].X = window->XStart + (window->XStart - p[i].X);
				if(p[i].X > window->XEnd)
					p[i].X = window->XEnd - (p[i].X - window->XEnd);
				
				if(p[i].Y < window->YStart)
					p[i].Y = window->YStart + (window->YStart - p[i].Y);
				if(p[i].Y > window->YEnd)
					p[i].Y = window->YEnd - (p[i].Y - window->YEnd);
				
				//cout<<p[i].X<<" "<<p[i].Y<<endl;
			}
			
			//for each iteration we calculate recent LOCAL_MIN. So we should remove the value that was generated in the previous iteration
			thread_local_min[th_id].localMin = 100000;			
			
			//Finding Local_Min of the thread
			for(int i=start; i<end; i++)
			{
				float func = F(p[i].X , p[i].Y);
			
				if(func < thread_local_min[th_id].localMin)
				{
					thread_local_min[th_id].localX = p[i].X;	
					thread_local_min[th_id].localY = p[i].Y;	
					thread_local_min[th_id].localMin = func;	
				}
			}
		});
		
		//-----------------------------Barrier---------------------------------//
		//Finding Local_Min and Global_Min
		for(int i=0; i<nw; i++)
		{		
			if(thread_local_min[i].localMin < local->localMin)
			{
				local->localX = thread_local_min[i].localX;	
				local->localY = thread_local_min[i].localY;	
				local->localMin = thread_local_min[i].localMin;	
			
				if(local->localMin < global->globalMin)
				{
					global->globalX	= local->localX;	
					global->globalY = local->localY;	
					global->globalMin = local->localMin;
				}
			}
		}		
	}
		
	return global->globalMin;
};


int main(int argc, char * argv[])
{
	int 	N = 100000;				//number of particles
	int 	NIteration = 10000;		//number of iterations
	float 	a = 0.1;
	float 	b = 0.2;
	float 	c = 0.1;
	int 	nw;  
	

	if(argc == 7)
	{	
		N = atoi(argv[1]);			//number of particles
		NIteration = atoi(argv[2]);	//number of iterations
		a = atoi(argv[3]);
		b = atoi(argv[4]);
		c = atoi(argv[5]);
		nw = atoi(argv[6]);
	}
    else if(argc == 2)
	{
		nw = atoi(argv[1]);
	}
	else
	{
		std::cerr << "use: " << argv[0]  
				<< " N NIteration a b c nw\n";
        return -1;
    }

	Particle 	*p		 = new Particle[N];
	Local_Min 	*local	 = new Local_Min;
	Global_Min 	*global	 = new Global_Min;
	Window		*window  = new Window;
	
	vector<Global_Min> threadGlobalMin(nw);	//for saving local minimum per thread	
	
	float _seed = time(NULL);
	
	//start program
	auto _start = std::chrono::high_resolution_clock::now();
	float result = iteration(_seed, ref(threadGlobalMin), N, NIteration, a,b,c, nw, ref(p),ref(local),ref(global), ref(window));	
	auto elapsed = std::chrono::high_resolution_clock::now() - _start;
	auto usec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	//end of program
	
	std::cout << "The result of SPO is: " << result << endl<<endl;;
	std::cout<<"Time for computing: "<<usec<<" microseconds"<<std::endl;

	return 0;
}