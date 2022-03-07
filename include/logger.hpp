#pragma once

#include <iostream>
#include <stdio.h>
#include <mutex>
#include <ctime>

enum LogPriority
{
    TracePriority, DebugPriority, InfoPriority, WarnPriority,
    ErrorPriority, CriticalPriority
};

class Logger
{
private:
    LogPriority priority = InfoPriority;
    std::mutex log_mutex;
    const char* filepath = 0;
    FILE* file = 0;
public:
    static void Set_Priority(LogPriority new_priority)
    {
	get_instance().priority = new_priority;
    }

    static void EnableFileOutput()
    {
	Logger& logger_instance = get_instance();
	logger_instance.filepath = "./log.txt";
	logger_instance.enable_file_output();
    }

    static void EnableFileOutput(const char* new_filepath)
    {
	Logger& logger_instance = get_instance();
	logger_instance.filepath = new_filepath;
	logger_instance.enable_file_output();
    }
    
    template<typename... Args>
    static void Trace(const char* message,Args... args)
    {
	get_instance().log("[Trace]\t",TracePriority,message,args...);
    }

    template<typename... Args>
    static void Debug(const char* message,Args... args)
    {
	get_instance().log("[Debug]\t",DebugPriority,message,args...);
    }
    template<typename... Args>
    static void Info(const char* message,Args... args)
    {
	get_instance().log("[Info]\t",InfoPriority,message,args...);
    }
    template<typename... Args>
    static void Warn(const char* message,Args... args)
    {
	get_instance().log("[Warn]\t",WarnPriority,message,args...);
    }
    template<typename... Args>
    static void Error(const char* message,Args... args)
    {
	get_instance().log("[Error]\t",ErrorPriority,message,args...);
    }
    template<typename... Args>
    static void Critical(const char* message,Args... args)
    {
	get_instance().log("[Critical]\t",CriticalPriority,message,args...);
    }

    template<typename... Args>
    static void Trace(const char* source_file,const char* function_name,const char* message,Args... args)
    {
	get_instance().log(source_file,function_name,"[Trace]\t",TracePriority,message,args...);
    }

    template<typename... Args>
    static void Debug(const char* source_file,const char* function_name,const char* message,Args... args)
    {
	get_instance().log(source_file,function_name,"[Debug]\t",DebugPriority,message,args...);
    }
    template<typename... Args>
    static void Info(const char* source_file,const char* function_name,const char* message,Args... args)
    {
	get_instance().log(source_file,function_name,"[Info]\t",InfoPriority,message,args...);
    }
    template<typename... Args>
    static void Warn(const char* source_file,const char* function_name,const char* message,Args... args)
    {
	get_instance().log(source_file,function_name,"[Warn]\t",WarnPriority,message,args...);
    }
    template<typename... Args>
    static void Error(const char* source_file,const char* function_name,const char* message,Args... args)
    {
	get_instance().log(source_file,function_name,"[Error]\t",ErrorPriority,message,args...);
    }
    template<typename... Args>
    static void Critical(const char* source_file,const char* function_name,const char* message,Args... args)
    {
	get_instance().log(source_file,function_name,"[Critical]\t",CriticalPriority,message,args...);
    }
private:
    Logger() {}

    Logger(const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;

    ~Logger()
    {
		if(file){
			free_file();
		}
    }
    
    static Logger& get_instance()
    {
	static Logger logger;
	return logger;
    }

    template<typename... Args>
    void log(const char* message_priority_str,LogPriority message_priority,
		    const char* message, Args... args)
    {
	std::time_t current_time = std::time(0);
	std::tm* timestamp = std::localtime(&current_time);
	char buffer[80];
	strftime(buffer,80,"%c",timestamp);

	if(priority <= message_priority){
	    std::scoped_lock lock(log_mutex);
	    if(file)
	    {
		fprintf(file,"%s\t",buffer);
		fprintf(file,message_priority_str);
		fprintf(file,message, args...);
		fprintf(file,"\n");
	    }else{
		printf("%s\t",buffer);
		printf(message_priority_str);
		printf(message, args...);
		printf("\n");
	    }
	}
    }

    template<typename... Args>
    void log(const char* source_file,const char* function_name,const char* message_priority_str,LogPriority message_priority,
		    const char* message, Args... args)
    {
	//std::time_t current_time = std::time(0);
	//std::tm* timestamp = std::localtime(&current_time);
	//char buffer[80];
	//strftime(buffer,80,"%c",timestamp);

	if(priority <= message_priority){
	    std::scoped_lock lock(log_mutex);
	    //printf("%s\t",buffer);
	    printf(message_priority_str);
	    printf(message, args...);
	    printf("\t\ton %s\t\tin %s",function_name,source_file);
	    printf("\n");
	    if(file)
	    {
		//fprintf(file,"%s\t",buffer);
		fprintf(file,message_priority_str);
		fprintf(file,message, args...);
		fprintf(file,"\t\ton %s\t\t in %s",function_name,source_file);
		fprintf(file,"\n");
	    }
	}
    }

    void enable_file_output()
    {
	if (file != 0)
	{
	    fclose(file);
	}
	file = fopen(filepath,"a");
	if(file==0)
	{
	    printf("Logger: Failed to open file at %s",filepath);
	}
    }

    void free_file()
    {
	fclose(file);
	file = 0;
    }
};


#define LOG_TRACE(Message, ...) (Logger::Trace(__FILE__,__FUNCTION__,Message,##__VA_ARGS__))
#define LOG_DEBUG(Message, ...) (Logger::Debug(__FILE__,__FUNCTION__,Message,##__VA_ARGS__))
#define LOG_INFO(Message, ...) (Logger::Info(__FILE__,__FUNCTION__,Message,##__VA_ARGS__))
#define LOG_WARN(Message, ...) (Logger::Warn(__FILE__,__FUNCTION__,Message,##__VA_ARGS__))
#define LOG_ERROR(Message, ...) (Logger::Error(__FILE__,__FUNCTION__,Message,##__VA_ARGS__))
#define LOG_CRITICAL(Message, ...) (Logger::Critical(__FILE__,__FUNCTION__,Message,##__VA_ARGS__))

