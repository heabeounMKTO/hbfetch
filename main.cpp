#include <cstdint>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

typedef struct MemoryInfo {
  float AvailableMem;
  float AllMem;
} MemoryInfo;

typedef struct CpuInfo {
  std::string CpuName;
  std::string CpuCore; 
  std::string CpuVendor;
  
} CpuInfo;

typedef struct UpTime {
  int Hours;
  int Minutes;
  int Seconds;
} UpTime;

typedef struct PcInfo {
  std::string HwVendor; 
  std::string HwModel;
  std::string Os; 
} OsInfo;


PcInfo get_os_info() {
	constexpr const char* os_release 	= "/etc/os-release";
	
  PcInfo pc_info; 
	auto ifs = std::ifstream{os_release};
	if(!ifs.good()){
		throw std::runtime_error("Error: unable to access /etc/os-release file.");
	}
	std::string line, label;
	std::uint64_t value; 
  std::string delimiter = "=";
	while( std::getline(ifs, line) )
	{		
    size_t pos = line.find(delimiter);
        if (pos != std::string::npos) {
            // Extract the key and value
            std::string field = line.substr(0, pos);
            std::string value = line.substr(pos + delimiter.length());

            // Trim quotes from the value if they exist
            if (value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }

            // Check if the field is "PRETTY_NAME"
            if (field == "PRETTY_NAME") {
                pc_info.Os = value;
                break;
            }
        }
	}
  return pc_info;
}

CpuInfo get_cpu_info() {
  constexpr const char* cpuinfo_file = "/proc/cpuinfo";
  CpuInfo cpu_info; 
  auto ifs = std::ifstream{cpuinfo_file};
  if (!ifs.good()) {
    throw std::runtime_error("Error: unable to access cpu-info file");
  }
  std::string line, label;
  std::uint64_t value;
  std::string delimiter = ":";
  while(std::getline(ifs, line)) {
        // Find the position of the delimiter
        size_t pos = line.find(delimiter);
        if (pos != std::string::npos) {
            // Extract the key and value
            std::string field = line.substr(0, pos);
            std::string value = line.substr(pos + delimiter.length());

            field.erase(field.find_last_not_of(" \t\n\r\f\v") + 1);
            // Trim whitespace from the value
            value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));

            if (field == "model name") {
                cpu_info.CpuName = std::string(value);
                break;
            }
            if (field == "cpu cores") { 
                cpu_info.CpuCore = std::string(value);
                break;
            }
        }
  }  
  return cpu_info;
}

std::string get_display_manager() {
  FILE *in;
  char buff[128];
  char *command = "echo $XDG_SESSION_TYPE";
  if ( !(in = popen(command, "r"))){
    throw std::runtime_error("error: unable to access sessions");  
  }
  std::stringstream ss; 
  fgets(buff, sizeof(buff), in);
  std::string _buff = std::string(buff);
  _buff.erase(_buff.find_last_not_of(" \t\n\r\f\v") + 1);
  return _buff;
}

std::string get_gpus() {
 FILE *in;
  char buff[128];
  char *command = "lspci | grep 'VGA'";
  if ( !(in = popen(command, "r"))){
    throw std::runtime_error("error: unable to access sessions");  
  }
  std::stringstream ss; 
  fgets(buff, sizeof(buff), in);
  std::string _buff = std::string(buff);
  _buff.erase(_buff.find_last_not_of(" \t\n\r\f\v") + 1);
  return _buff;
}


/// returns uptime in seconds
UpTime get_sys_uptime() {
  constexpr const char* uptime = "/proc/uptime";
  auto ifs = std::ifstream{uptime};
  UpTime system_uptime;
  if(!ifs.good()) {
    throw std::runtime_error("Error: unable to access uptime file");
  }
  std::string line;
  std::uint64_t fuck;
  while (std::getline(ifs, line)) {
    std::stringstream ss{line};
    ss >> fuck;
  }
  float hours = fuck/3600.0;
  float minutes = 60 * (hours - (int)hours);

  system_uptime.Hours = (int)(hours);
  system_uptime.Minutes = (int)(minutes);
  return system_uptime;
}

MemoryInfo get_memory_info()
{
	constexpr const char* meminfo_file 		= "/proc/meminfo";
	constexpr double factor = 1024 * 1024;
	
  MemoryInfo m_info;
	auto ifs = std::ifstream{meminfo_file};
	if(!ifs.good()){
		throw std::runtime_error("Error: unable to access memory-info file.");
	}
	std::string line, label;
	std::uint64_t value; 
	while( std::getline(ifs, line) )
	{		
		std::stringstream ss{line};	
		ss >> label >> value;

    if(label == "MemTotal:") {
     m_info.AllMem = value/factor; 
    }

		if(label == "MemAvailable:")		
     m_info.AvailableMem = value/factor; 
	}
  return m_info;
}

void pretty_print_sys_info(utsname* sysinfo, 
                           MemoryInfo* mem_info,
                           UpTime* system_uptime,
                           PcInfo* pc_info,
                           CpuInfo* cpu_info) {
  std::string display_manager = get_display_manager(); 
  std::ostringstream pretty_info;
  pretty_info << R"(
                  @@@@@@                                 
                @@@@@@@@@@               
               @@@@@@@@@@@                    __h(ea)b(eoun's)fetch___ 
         @@@@@@@@@@@@@@@@@@@@@@               node_name: )" << sysinfo->nodename << R"( 
       @@@@@@@@@@@@@@@@@@@@@@@@@@@@           architecture: )" << sysinfo->machine << R"(
      @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@          kernel: )" << sysinfo->release << R"(
       @@@@@@@@@@        @@@@@@@@@@@          memory: )" << mem_info->AllMem - mem_info->AvailableMem <<"/"<< mem_info->AllMem << " GB *used/available*" << R"(  
        @@@@@@@@          @@@@@@@@@           uptime: )" << system_uptime->Hours << " hours " << system_uptime->Minutes << " minutes" << R"(
          @@@@@@          @@@@@@@             operating system: )" << pc_info->Os << R"(
       @@@@@@@@@@        @@@@@@               display protocol: )" << display_manager << R"(
     @@@@@@@@@@@@@@@@@@@@@@@@@@@@             cpu: )" << cpu_info->CpuName << cpu_info->CpuCore << " cores" << R"( 
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@           
   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@        
   @@@@@@@@@@@@@@@    @@@@@@@@@@@@@@        
    @@@@@@@@@@@@        @@@@@@@@@@@         
     @@@@@@@@@             @@@@@@ )";          
  std::cout << pretty_info.str() << std::endl;
}

int main() {
  MemoryInfo fucc = get_memory_info();
  CpuInfo cpu_info = get_cpu_info();
  // printf("ayylmao %s", fucc2.CpuName);
  struct utsname sysinfo;
  uname(&sysinfo);
  PcInfo pc_info = get_os_info();
  UpTime sysuptime = get_sys_uptime();
  // std::cout << fucc2.CpuName << std::endl;
  pretty_print_sys_info(&sysinfo, &fucc, &sysuptime, &pc_info, &cpu_info);
  return 0;
}
