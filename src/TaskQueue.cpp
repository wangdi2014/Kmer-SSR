/*
	Brandon Pickett
	TaskQueue.cpp
*/

#include "../include/TaskQueue.h"

TaskQueue::TaskQueue()
{
	this->setUp(1000);
	//this->max_capacity = 1000;
}
TaskQueue::TaskQueue(uint32_t max_capacity)
{
	this->setUp(max_capacity);
	//this->max_capacity = size;
}
TaskQueue::~TaskQueue()
{
	this->clear();
	delete this->tasks;
}
void TaskQueue::setUp(uint32_t max_capacity)
{
	this->tasks = new queue<Task*>();
	sem_init(&(this->lock),0,1);
	sem_init(&(this->unfinished_business),0,0);
	sem_init(&(this->space_available),0,max_capacity);
}
uint32_t TaskQueue::size() const
{
	return this->tasks->size();
}
bool TaskQueue::empty() const
{
	return this->tasks->empty();
}
void TaskQueue::add(Task* task)
{
	sem_wait(&(this->space_available));
	sem_wait(&(this->lock));
	this->tasks->push(task);
	sem_post(&(this->lock));
	sem_post(&(this->unfinished_business));
}
Task* TaskQueue::get()
{
	sem_wait(&(this->unfinished_business));
	sem_wait(&(this->lock));
	Task* temp_ptr = this->tasks->front();
	this->tasks->pop();
	sem_post(&(this->lock));
	sem_post(&(this->space_available));
	return temp_ptr;
}
void TaskQueue::clear()
{
	sem_wait(&(this->lock));
	while (!this->tasks->empty())
	{
		delete this->tasks->front();
		this->tasks->pop();

		sem_wait(&(this->unfinished_business)); // this will never block (well, it shouldn't...if it does, that's a logic error)
		sem_post(&(this->space_available));
	}
	sem_post(&(this->lock));
}
string TaskQueue::toString()
{
	queue<Task*>* new_tasks = new queue<Task*>();

	string output = "TaskQueue: { ";

	sem_wait(&(this->lock));

	Task* front_task = this->tasks->front();
	
	while (!this->tasks->empty())
	{
		if (!front_task->equalTo(this->tasks->front()))
		{
			output += ", ";
		}
		
		output += this->tasks->front()->toString();
		
		new_tasks->push(this->tasks->front());
		this->tasks->pop();
	}

	output += " }";

	queue<Task*>* temp_ptr = this->tasks;
	this->tasks = new_tasks;
	delete temp_ptr;

	sem_post(&(this->lock));

	return output;
}
string TaskQueue::toJson()
{
	queue<Task*>* new_tasks = new queue<Task*>();

	string json = "TaskQueue: {";
	
	sem_wait(&(this->lock));
	
	Task* front_task = this->tasks->front();

	if (this->size() != 0)
	{
		json += "\n\t";
		
		while (!this->tasks->empty())
		{
			if (!front_task->equalTo(this->tasks->front()))
			{
				json += ",\n\t";
			}
			
			json += this->tasks->front()->toString();;

			new_tasks->push(this->tasks->front());
			this->tasks->pop();
		}

		json += '\n';
	}
	else
	{
		json += ' ';
	}

	json += '}';

	queue<Task*>* temp_ptr = this->tasks;
	this->tasks = new_tasks;
	delete temp_ptr;
	
	sem_post(&(this->lock));

	return json;
}
