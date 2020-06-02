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
#include <mutex>
#include <atomic>


using namespace std;

mutex my_mutex;
pthread_barrier_t my_barrier;

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

void init(int i_th, float _seed, int start, int end, int N, int NIteration, float a,float b,float c, Particle *p, Local_Min *local, Local_Min thread_local_min[], Global_Min *global, Window *window)
{	
	//Giving starting point for each particle
	//cout<<"The initial points of Particles"<<endl;
	for(int i=start; i<end; i++)
	{
		p[i].X = RandomNumberGenerator(_seed, window->XStart, window->XEnd);
		p[i].Y = RandomNumberGenerator(_seed, window->XStart, window->XEnd);		
		//cout << p[i].X << " " << p[i].Y << endl;
	}
	
	//Giving initial speed for each Particle
	for(int i=start; i<end; i++)
	{
		p[i].speedX = RandomNumberGenerator(_seed, -10, 10);
		p[i].speedY = RandomNumberGenerator(_seed, -10, 10); 		
		//cout<<"Initial speeds of "<<i<<"th partical: "<<p[i].speedX<<" "<<p[i].speedY<<endl;
	}
	
	//Giving initial value for Local_Min and Global_Min
	for(int i=start; i<end; i++)
	{
		float func = F(p[i].X, p[i].Y);
		
		if(func < thread_local_min[i_th].localMin)
		{
			thread_local_min[i_th].localX = p[i].X;	
			thread_local_min[i_th].localY = p[i].Y;	
			thread_local_min[i_th].localMin = func;		
		}
	}
	
	//cout<<"thread_local_min of "<<i_th<<" is "<<thread_local_min[i_th].localMin<<endl;
	
	//-----------------------------Barrier---------------------------------//
	/*
	pthread_barrier_wait(&my_barrier);
	for(int i=start; i<end; i++)
	{
		float func = F(p[i].X, p[i].Y);
		
		std::lock_guard<std::mutex> lock(my_mutex);
		if(func < local->localMin)
		{		
			local->localX = p[i].X;	
			local->localY = p[i].Y;	
			local->localMin = F(local->localX , local->localY);		
			
			if(func < global->globalMin)
			{
				global->globalX = p[i].X;	
				global->globalY = p[i].Y;	
				global->globalMin = func;
			}	
		}
	}
	*/
}

void iteration(int i_th, float _seed, int start, int end, int N, int NIteration, float a,float b,float c, Particle *p, Local_Min *local, Local_Min thread_local_min[], Global_Min *global, Window *window)
{
	float velocity;
	
	//for each iteration we calculate recent Local_Min. So we should remove the value that was generated in the previous iteration
	thread_local_min[i_th].localMin = 100000;
	
	//Updating velocity
	//cout<<"The new velocity of Particles"<<endl
	for(int i=start; i<end; i++)
	{
		velocity  =	a * p[i].speedX + 
					b * ((float) RandomNumberGenerator(_seed, 0, 1)/RAND_MAX) * (p[i].X - local->localX) +
					c * ((float) RandomNumberGenerator(_seed, 0, 1)/RAND_MAX) * (p[i].X - global->globalX);
					
		p[i].speedX = (velocity > 10) ? 10 : ((velocity < -10) ? -10 : velocity);
		
		velocity  =	a * p[i].speedY + 
					b * ((float) RandomNumberGenerator(_seed, 0, 1)/RAND_MAX) * (p[i].Y - local->localY) +
					c * ((float) RandomNumberGenerator(_seed, 0, 1)/RAND_MAX) * (p[i].Y - global->globalY);
		
		
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
	
	//Finding Local_Min of the thread
	for(int i=start; i<end; i++)
	{
		float func = F(p[i].X, p[i].Y);
		
		if(func < thread_local_min[i_th].localMin)
		{
			thread_local_min[i_th].localX = p[i].X;	
			thread_local_min[i_th].localY = p[i].Y;	
			thread_local_min[i_th].localMin = func;		
		}
	}
	
	//-----------------------------Barrier---------------------------------//
	/*
	pthread_barrier_wait(&my_barrier);
	for(int i=start; i<end; i++)
	{
		float func = F(p[i].X, p[i].Y);
		
		std::lock_guard<std::mutex> lock(my_mutex);
		if(func < local->localMin)
		{		
			local->localX = p[i].X;	
			local->localY = p[i].Y;	
			local->localMin = F(local->localX , local->localY);		
			
			if(func < global->globalMin)
			{
				global->globalX = p[i].X;	
				global->globalY = p[i].Y;	
				global->globalMin = func;
			}	
		}
	}
	*/
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
	
	Local_Min 	*thread_local_min	 = new Local_Min[nw];//for saving local minimum per thread in each iteration
	vector<thread*> myThread(nw);
	pthread_barrier_init(&my_barrier, NULL, nw);
	
	//start program
	auto _start = std::chrono::high_resolution_clock::now();
	int start;
	int end=0;  
	int NTask = N / nw;
	int more = N % nw;
	
	float _seed = time(NULL);
	cout.precision(15);	
	//cout<<"_seed = "<<_seed<<endl;
	
	float seed_per_thread[nw];
	for(int i=0; i<nw; i++)
	{
		seed_per_thread[i] = RandomNumberGenerator(_seed, 1000000000, _seed);
		//cout<<"seed_per_thread = "<<seed_per_thread[i]<<endl;
	}
	
	
	for(int i_th=0; i_th<nw; i_th++)
	{
		//calculating the starting and end points of the chunk for each thread
		start = end;
		if(more!=0)
		{
			end	= start + NTask + 1;
			more--;
		}
		else
		{
			end	= start + NTask;
		}
		myThread[i_th] = new thread(init, i_th, seed_per_thread[i_th], start, end, N, NIteration, a,b,c, ref(p),ref(local), thread_local_min, ref(global), ref(window));
	}
	
	for(int i_th=0; i_th<nw; i_th++)
		myThread[i_th]->join();
	
	for(int i_th=0; i_th<nw; i_th++)
	{
		if(thread_local_min[i_th].localMin < local->localMin)
		{
			local->localX = thread_local_min[i_th].localX;	
			local->localY = thread_local_min[i_th].localY;	
			local->localMin = thread_local_min[i_th].localMin;	
			
			if(local->localMin < global->globalMin)
			{
				global->globalX = thread_local_min[i_th].localX;	
				global->globalY = thread_local_min[i_th].localY;	
				global->globalMin = thread_local_min[i_th].localMin;
			}
		}	
	}
	
	// Starting iterations
	for(int it=0; it<NIteration; it++)
	{
		//calculating the starting and end points of the chunk for each thread
		end=0;  
		NTask = N / nw;
		more = N % nw;
		for(int i_th=0; i_th<nw; i_th++)
		{
			start = end;
			if(more!=0)
			{
				end	= start + NTask + 1;
				more--;
			}
			else
			{
				end	= start + NTask;
			}
			myThread[i_th] = new thread(iteration, i_th, seed_per_thread[i_th], start, end, N, NIteration, a,b,c, ref(p), ref(local), thread_local_min, ref(global), ref(window));
		}
		
		for(int i_th=0; i_th<nw; i_th++)
			myThread[i_th]->join();
	
		for(int i_th=0; i_th<nw; i_th++)
		{
			if(thread_local_min[i_th].localMin < local->localMin)
			{
				local->localX = thread_local_min[i_th].localX;	
				local->localY = thread_local_min[i_th].localY;	
				local->localMin = thread_local_min[i_th].localMin;	
				
				if(local->localMin < global->globalMin)
				{
					global->globalX = thread_local_min[i_th].localX;	
					global->globalY = thread_local_min[i_th].localY;	
					global->globalMin = thread_local_min[i_th].localMin;
				}
			}
		}
		
	}
	
	
	auto elapsed = std::chrono::high_resolution_clock::now() - _start;
	auto usec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	//end of program
	
	std::cout << "The result of SPO is: " << global->globalMin << endl<<endl;;
	std::cout<<"Time for computing: "<<usec<<" microseconds"<<std::endl;

	return 0;
}