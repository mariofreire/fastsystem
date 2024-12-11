#include "enum.h"


// Vendor strings from CPUs.
#define CPUID_VENDOR_AMD           "AuthenticAMD"
#define CPUID_VENDOR_AMD_OLD       "AMDisbetter!"
#define CPUID_VENDOR_INTEL         "GenuineIntel"
#define CPUID_VENDOR_VIA           "VIA VIA VIA "
#define CPUID_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPUID_VENDOR_TRANSMETA_OLD "TransmetaCPU"
#define CPUID_VENDOR_CYRIX         "CyrixInstead"
#define CPUID_VENDOR_CENTAUR       "CentaurHauls"
#define CPUID_VENDOR_NEXGEN        "NexGenDriven"
#define CPUID_VENDOR_UMC           "UMC UMC UMC "
#define CPUID_VENDOR_SIS           "SiS SiS SiS "
#define CPUID_VENDOR_NSC           "Geode by NSC"
#define CPUID_VENDOR_RISE          "RiseRiseRise"
#define CPUID_VENDOR_VORTEX        "Vortex86 SoC"
#define CPUID_VENDOR_AO486         "MiSTer AO486"
#define CPUID_VENDOR_AO486_OLD     "GenuineAO486"
#define CPUID_VENDOR_ZHAOXIN       "  Shanghai  "
#define CPUID_VENDOR_HYGON         "HygonGenuine"
#define CPUID_VENDOR_ELBRUS        "E2K MACHINE "

// Vendor strings from hypervisors.
#define CPUID_VENDOR_QEMU          "TCGTCGTCGTCG"
#define CPUID_VENDOR_KVM           " KVMKVMKVM  "
#define CPUID_VENDOR_VMWARE        "VMwareVMware"
#define CPUID_VENDOR_VIRTUALBOX    "VBoxVBoxVBox"
#define CPUID_VENDOR_XEN           "XenVMMXenVMM"
#define CPUID_VENDOR_HYPERV        "Microsoft Hv"
#define CPUID_VENDOR_PARALLELS     " prl hyperv "
#define CPUID_VENDOR_PARALLELS_ALT " lrpepyh vr "
#define CPUID_VENDOR_BHYVE         "bhyve bhyve "
#define CPUID_VENDOR_QNX           " QNXQVMBSQG "

// Vendor id from CPUs.
#define VENDOR_INTEL      1
#define VENDOR_UMC        2
#define VENDOR_AMD        3
#define VENDOR_CYRIX      4
#define VENDOR_NEXGEN     5
#define VENDOR_CENTAUR    6
#define VENDOR_RISE       7
#define VENDOR_SIS	  	  8
#define VENDOR_TRANSMETA  9
#define VENDOR_NSC	     10
#define VENDOR_HYGON	 11
#define VENDOR_ZHAOXIN   12
#define VENDOR_UNKNOWN   99

// CPU vendor id string from CPUs.
#define CPUID_ID_INTEL      	"Intel"
#define CPUID_ID_UMC        	"UMC"
#define CPUID_ID_AMD        	"AMD"
#define CPUID_ID_CYRIX      	"Cyrix"
#define CPUID_ID_NEXGEN     	"NexGen"
#define CPUID_ID_CENTAUR    	"Centaur"
#define CPUID_ID_RISE       	"Rise"
#define CPUID_ID_SIS	  	  	"SiS"
#define CPUID_ID_TRANSMETA  	"Transmeta"
#define CPUID_ID_NSC	     	"NSC"
#define CPUID_ID_HYGON	 		"Hygon"
#define CPUID_ID_ZHAOXIN   		"Zhaoxin"
#define CPUID_ID_UNKNOWN   		"x86"
#define CPUID_ID_GENERIC_X86 	"x86"
#define CPUID_ID_GENERIC_X64	"x64"


#define UCHAR8A(value) ((unsigned char)(value))
#define UCHAR8B(value) ((unsigned char)((value)>> 8))
#define UCHAR8C(value) ((unsigned char)((value)>>16))
#define UCHAR8D(value) ((unsigned char)((value)>>24))

#define UINT32(a,b,c,d) ((unsigned long)((unsigned char)(a)|((unsigned char)(b)<<8)|((unsigned char)(c)<<16)|((unsigned char)(d)<<24)))

#define NULL 0LL

#define HEAP_START 0xC00000
#define HEAP_END 0x1800000
#define PAGE_SIZE 0x1000
#define ALLOC_SIZE_HEADER  8

#pragma pack (push, 1)


typedef struct
{
	char signature[4];
	unsigned long version;
	unsigned short flags;
	unsigned char unused;
	unsigned long multi_boot;
	unsigned long heap_1_size;
	unsigned long heap_2_size;
	unsigned long heap_3_size;
	unsigned long heap_4_size;
	unsigned long physical_memory;
	unsigned long cpu_speed;
	unsigned long cpu_id_family;
	unsigned long cpu_id_model;
	unsigned long cpu_id_stepping;
	unsigned long cpu_id_type;
	unsigned char cpu_id_longmode;
	char cpu_id_name[32];
	char cpu_id_str[64];
	char cpu_brand_str[256];
	char reserved[104];
} system_info_t;

typedef struct
{
	char signature[4];
	unsigned short version;
	unsigned char reserved;
	unsigned char machine;
	unsigned short flags;
	unsigned long count;
	unsigned long checksum;
	unsigned long key;
	unsigned long crc;
	char id[38];
} sys_vars_info_t;

typedef struct
{
	char signature[4];
	unsigned short version;
	unsigned char reserved;
	unsigned char machine;
	unsigned short flags;
	unsigned long count;
	unsigned long checksum;
	unsigned long key;
	unsigned long crc;
	char id[38];
} sys_enum_info_t;

#pragma pack (pop)

typedef long unsigned int size_t;

void * _heap_start;
void * _heap_end;
void * _heap_current;

unsigned long _heap_last_size_alloc;
unsigned long _heap_last_position;
unsigned long _heap_position;
unsigned long _heap_size;

unsigned long _heap_alloc_last_clean_start;
unsigned long _heap_alloc_last_clean_end;

unsigned char *system_info = (unsigned char *)0x840000;
unsigned char *system_variables_info = (unsigned char *)0x808000;
unsigned char *system_variables_enum_info = (unsigned char *)0x818000;

system_info_t *info;
sys_vars_info_t *sys_vars_info;
sys_enum_info_t *sys_enum_info;

int sys_vars_loaded = 0;
int sys_enum_loaded = 0;

unsigned long cpu_speed = 100;

static unsigned cycles_high = 0, cycles_low = 0;

extern unsigned long probememory(void);

void* memcpy(void *s1, const void *s2, size_t n)
{
	char *dest = s1;
	const char *source = s2;
	while (n--)
	{
		*dest++ = *source++;
	}
	return s1;
}

void* memset(void *s, int c, size_t n)
{
	unsigned char *dest = s;
	while (n--)
	{
		*dest++ = (unsigned char)c;
	}
	return s;
}

char *strcpy(char *s1, const char *s2)
{
	char *dest = s1;
	while ((*dest++ = *s2++) != 0);
	return s1;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2)
	{
		if (!*s1)
		{
			return 0;
		}
		s1++;
		s2++;
	}
	return *(unsigned const char*)s1 - *(unsigned const char*)s2;	
}

char *strcat(char *s1, const char *s2)
{
	char *dest = s1;
	while (*dest)
	{
		*dest++;
	}
	while ((*dest++ = *s2++) != 0);
	return s1;
}

size_t strlen(const char *s)
{
	const char *s1 = s;
	if (s == NULL)
	{
		return 0;
	}
	for (s1 = s; *s1; s1++);
	return (s1 - s);
}

static void get_cpu_timestamp(unsigned *high, unsigned *low)
{
    __asm__ volatile ("rdtsc; movl %%edx,%0; movl %%eax,%1"
					: "=r" (*high), "=r" (*low)
					:
					: "%edx", "%eax");
}

void start_cpu_counter(void)
{
    get_cpu_timestamp(&cycles_high, &cycles_low);
}

double get_cpu_counter(void)
{
    unsigned nh, nl, h, l, b;
    double r;
    get_cpu_timestamp(&nh, &nl);
    l = nl - cycles_low;
    b = l > nl;
    h = nh - cycles_high - b;
    r = (double) h * (1 << 30) * 4 + l;
    return r;
}

void delay_1(void)
{
	int i=0, j=0, k=0;
	while (i<(1024*1024))
	{
		j=0;
		while (j<(1024*1024))
		{
			k=0;
			while (k<(1024*1024))
			{
				k++;
			}
			j++;
		}
		i++;
	}
}

unsigned long get_cpu_speed(void)
{
	unsigned long f=0;
    double c1, c2;
	double ff;
    start_cpu_counter();
    c1 = get_cpu_counter();
    delay_1();
    c2 = get_cpu_counter();
	ff = (c2-c1)/1E6;
	f = (unsigned long)(ff);
	return f;
}

void cpuid(int c, unsigned long *a, unsigned long *d)
{
	__asm__ volatile ("cpuid"
				  :"=a"(*a),
				   "=d"(*d)
				  :"a"(c)
				  :"ecx",
				  "ebx");
}

int getcpuid(int c, unsigned long d[4])
{
	__asm__ volatile ("cpuid"
				  :"=a"(*d),
				   "=b"(*(d+1)),
				   "=c"(*(d+2)),
				   "=d"(*(d+3))
				  :"a"(c));				  
	return (int)d[0];
}

void dwordtobytes(unsigned long dw, unsigned char bytes[4])
{
    unsigned char *c;
    c = (unsigned char*)&dw;
    bytes[0] = c[3];
    bytes[1] = c[2];
    bytes[2] = c[1];
    bytes[3] = c[0];
    return;
}

unsigned long swap32( unsigned long value )
{
    value = ((value << 8) & 0xFF00FF00 ) | ((value >> 8) & 0xFF00FF ); 
    return (value << 16) | (value >> 16);
}

unsigned char has_cpu_support_longmode(void)
{
	unsigned long d[4];
	unsigned char has_longmode = 0;
	getcpuid(0x80000001, d);
	has_longmode = ((d[3] >> 29) & 0x01);
	return has_longmode;
}

void get_cpu_vendor_string(char *s)
{
  unsigned long d[4];
  union
  {
	char c[16];
	int  i[4];
  } v;
  getcpuid(0, d);
  *(&v.i[0]) = d[1];
  *(&v.i[1]) = d[3];
  *(&v.i[2]) = d[2];
  v.c[12] = '\0';
  strcpy(s, v.c);
}


int get_cpu_vendor(void)
{
	char vendor[16];
	unsigned long d[4];
	union
	{
	char c[16];
	int  i[4];
	} v;
	getcpuid(0, d);
	*(&v.i[0]) = d[1];
	*(&v.i[1]) = d[3];
	*(&v.i[2]) = d[2];
	v.c[12] = '\0';
	strcpy(vendor, v.c);
	if (strcmp(vendor, CPUID_VENDOR_INTEL) == 0)
	{
		return VENDOR_INTEL;
	}
	else if (strcmp(vendor, CPUID_VENDOR_AMD) == 0)
	{
		return VENDOR_AMD;
	}
	else if (strcmp(vendor, CPUID_VENDOR_UMC) == 0)
	{
		return VENDOR_UMC;
	}
	else if (strcmp(vendor, CPUID_VENDOR_CYRIX) == 0)
	{
		return VENDOR_CYRIX;
	}
	else if (strcmp(vendor, CPUID_VENDOR_NEXGEN) == 0)
	{
		return VENDOR_NEXGEN;
	}
	else if (strcmp(vendor, CPUID_VENDOR_SIS) == 0)
	{
		return VENDOR_SIS;
	}
	else if (strcmp(vendor, CPUID_VENDOR_RISE) == 0)
	{
		return VENDOR_RISE;
	}
	else if (strcmp(vendor, CPUID_VENDOR_CENTAUR) == 0)
	{
		return VENDOR_CENTAUR;
	}
	else if (strcmp(vendor, CPUID_VENDOR_HYGON) == 0)
	{
		return VENDOR_HYGON;
	}
	else if (strcmp(vendor, CPUID_VENDOR_NSC) == 0)
	{
		return VENDOR_NSC;
	}
	else if (strcmp(vendor, CPUID_VENDOR_TRANSMETA) == 0)
	{
		return VENDOR_TRANSMETA;
	}
	else if (strcmp(vendor, CPUID_VENDOR_ZHAOXIN) == 0)
	{
		return VENDOR_ZHAOXIN;
	}
	else if (strcmp(vendor, CPUID_VENDOR_AMD_OLD) == 0)
	{
		return VENDOR_AMD;
	}
	else if (strcmp(vendor, CPUID_VENDOR_TRANSMETA_OLD) == 0)
	{
		return VENDOR_TRANSMETA;
	}
	if ((d[0] == 0) || ((d[0] & 0x500) != 0)) return VENDOR_INTEL;
	return VENDOR_UNKNOWN;
}


void get_cpu_vendor_id_string(char *s)
{
  int has_64_bits;
  int vendor_id;
  char id_str[32];
  unsigned long d[4];
  getcpuid(0, d);
  vendor_id = get_cpu_vendor();
  has_64_bits = has_cpu_support_longmode();
  switch(vendor_id)
  {
	  case VENDOR_INTEL:
	  {
		  strcpy(id_str, CPUID_ID_INTEL);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_AMD:
	  {
		  strcpy(id_str, CPUID_ID_AMD);
		  if (has_64_bits)
		  {
			  strcat(id_str, "64");
		  }
	  }
	  break;
	  case VENDOR_CYRIX:
	  {
		  strcpy(id_str, CPUID_ID_CYRIX);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_NEXGEN:
	  {
		  strcpy(id_str, CPUID_ID_NEXGEN);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_CENTAUR:
	  {
		  strcpy(id_str, CPUID_ID_CENTAUR);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_RISE:
	  {
		  strcpy(id_str, CPUID_ID_RISE);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_SIS:
	  {
		  strcpy(id_str, CPUID_ID_SIS);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_TRANSMETA:
	  {
		  strcpy(id_str, CPUID_ID_TRANSMETA);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_NSC:
	  {
		  strcpy(id_str, CPUID_ID_NSC);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_HYGON:
	  {
		  strcpy(id_str, CPUID_ID_HYGON);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_ZHAOXIN:
	  {
		  strcpy(id_str, CPUID_ID_ZHAOXIN);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_UNKNOWN:
	  {
		  if (has_64_bits)
		  {
			  strcpy(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcpy(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  default:
	  {
		  if (has_64_bits)
		  {
			  strcpy(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcpy(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
  };
  strcpy(s, id_str);
}


void get_cpu_str_eax(unsigned long eax, char *s)
{
  char str[17];
  unsigned long d[4];
  union
  {
	char c[16];
	int  i[4];
  } v;
  getcpuid(eax, d);
  *(&v.i[0]) = d[0];
  *(&v.i[1]) = d[1];
  *(&v.i[2]) = d[2];
  *(&v.i[3]) = d[3];
  strcpy(str, v.c);
  str[16] = '\0';
  strcpy(s, str);
}

void get_cpu_brand_str(char *s)
{
	char cpu_str[64];
	char cpu_str_1[16];
	char cpu_str_2[16];
	char cpu_str_3[16];
	get_cpu_str_eax(0x80000002,cpu_str_1);
	strcpy(cpu_str, cpu_str_1);
	get_cpu_str_eax(0x80000003,cpu_str_2);
	strcat(cpu_str, cpu_str_2);
	get_cpu_str_eax(0x80000004,cpu_str_3);
	strcat(cpu_str, cpu_str_3);
	strcpy(s, cpu_str);
}

void get_cpu_info(int *type, int *family, int *model, int *stepping)
{
	unsigned long d[4];
	getcpuid(1, d);
	*type = (((d[0] >> 12) & 0x03) + ((d[0] >> 12) & 0x0f));
	*family = (((d[0] >> 8) & 0x0f) + ((d[0] >> 20) & 0xff));
	*model = (((d[0] >> 4) & 0x0f));
	if (*model == 0)
	{
		*model = ((d[0] >> 12) & 0xf0);
	}
	*stepping = ((d[0]) & 0x0f);
}

void init_heap(void)
{
	_heap_last_size_alloc = 0;
	_heap_position = HEAP_START + PAGE_SIZE + ALLOC_SIZE_HEADER;
	_heap_last_position = _heap_position;
	_heap_start = (void*)HEAP_START;
	_heap_end = (void*)HEAP_END;
	_heap_current = (void*)_heap_position;
	_heap_size = HEAP_END-HEAP_START;
	_heap_alloc_last_clean_start = 0;
	_heap_alloc_last_clean_end = 0;
}

void *malloc(size_t size)
{
	unsigned char *alloc_header;
	unsigned long alloc_pos;
	unsigned long alloc_size;
	union hdr {
		struct
		{
			unsigned char d[4];
		};
		struct
		{
			unsigned long l;
		};
	};
	union hdr pos;
	union hdr siz;
	if (_heap_last_position < ((HEAP_START + PAGE_SIZE) - ALLOC_SIZE_HEADER)) return NULL;
	if (_heap_last_position >= (HEAP_END-ALLOC_SIZE_HEADER)) return NULL;
	if (size == 0) return NULL;
	_heap_position += _heap_last_size_alloc+ALLOC_SIZE_HEADER;
	alloc_pos = _heap_position;
	alloc_size = size;
	alloc_header = (unsigned char*)_heap_current-ALLOC_SIZE_HEADER;
	if ((size-_heap_last_size_alloc) < 0)
	{
		*alloc_header++ = UCHAR8A(alloc_pos);
		*alloc_header++ = UCHAR8B(alloc_pos);
		*alloc_header++ = UCHAR8C(alloc_pos);
		*alloc_header++ = UCHAR8D(alloc_pos);
		*alloc_header++ = UCHAR8A(alloc_size);
		*alloc_header++ = UCHAR8B(alloc_size);
		*alloc_header++ = UCHAR8C(alloc_size);
		*alloc_header++ = UCHAR8D(alloc_size);
	} else
	{
		pos.l = alloc_pos;
		siz.l = alloc_size;
		for(int i=0;i<4;i++) alloc_header[i] = pos.d[i];
		for(int i=0;i<4;i++) alloc_header[4+i] = siz.d[i];
	}
	_heap_current += _heap_last_size_alloc+ALLOC_SIZE_HEADER;
	_heap_last_size_alloc = size;
	_heap_last_position += _heap_last_size_alloc+ALLOC_SIZE_HEADER;
	return _heap_current;//-ALLOC_SIZE_HEADER;
}

void free(void *ptr)
{
	static unsigned char *alloc_ptr;
	const unsigned char *alloc_header;
	unsigned long alloc_pos;
	unsigned long alloc_size;
	union hdr {
		struct
		{
			unsigned char d[4];
		};
		struct
		{
			unsigned long l;
		};
	};
	union hdr pos;
	union hdr siz;
	alloc_ptr = (unsigned char*)ptr;
	alloc_header = (unsigned char*)ptr-ALLOC_SIZE_HEADER;
	unsigned char pos_a = *alloc_header++;
	unsigned char pos_b = *alloc_header++;
	unsigned char pos_c = *alloc_header++;
	unsigned char pos_d = *alloc_header++;
	unsigned char siz_a = *alloc_header++;
	unsigned char siz_b = *alloc_header++;
	unsigned char siz_c = *alloc_header++;
	unsigned char siz_d = *alloc_header++;
	alloc_pos = UINT32(pos_a, pos_b, pos_c, pos_d);
	alloc_size = UINT32(siz_a, siz_b, siz_c, siz_d);
	if (alloc_size == 0 || alloc_pos == 0)
	{
		for(int i=0;i<4;i++) pos.d[i] = alloc_header[i];
		for(int i=0;i<4;i++) siz.d[i] = alloc_header[i+4];
		alloc_pos = pos.l;
		alloc_size = siz.l;
	}
	//print_int(alloc_size);
	//putch('\n');
	int i;
	i = 0;
	while(i < alloc_size+ALLOC_SIZE_HEADER)
	{
		if (*alloc_ptr == 0) break;
		*alloc_ptr++ = 0;
		alloc_ptr--;
		i++;
	};
	//_heap_current -= alloc_size;
	//_heap_position -= alloc_size;
	_heap_alloc_last_clean_start = alloc_pos;
	_heap_alloc_last_clean_end = alloc_pos+alloc_size;
}

void setsignature(void *_info_, char *s)
{
	unsigned char *_info_s = (unsigned char*)_info_;
	_info_s[0] = s[0];
	_info_s[1] = s[1];
	_info_s[2] = s[2];
	_info_s[3] = s[3];
}


void loadsysteminfo(void)
{
	unsigned long physical_memory_l;
	unsigned long physical_memory;
	unsigned long used_memory;
	unsigned long used_memory_percent;
	char *cpu_id_name;
	char *cpu_id_str;
	char *cpu_brand_str;
	int cpu_id_type, cpu_id_family, cpu_id_model, cpu_id_stepping, cpu_id_longmode;
	char info_signature[] = "INFO";
	system_info = (unsigned char *)0x840000;

	cpu_speed = get_cpu_speed();
	get_cpu_info(&cpu_id_type, &cpu_id_family, &cpu_id_model, &cpu_id_stepping);
	cpu_id_longmode = has_cpu_support_longmode();

	physical_memory_l = probememory();
	physical_memory = physical_memory_l / 1024 / 1024;
	used_memory = _heap_size / 1024 / 1024;
	used_memory_percent = (100/(physical_memory/used_memory));

	cpu_id_name = (char*)malloc(32);
	cpu_id_str = (char*)malloc(64);
	cpu_brand_str = (char*)malloc(256);

	memset(cpu_id_name, 0, 32);
	memset(cpu_id_str, 0, 64);
	memset(cpu_brand_str, 0, 256);
	
	get_cpu_vendor_id_string(cpu_id_name);
	get_cpu_vendor_string(cpu_id_str);
	get_cpu_brand_str(cpu_brand_str);
	
	info = (system_info_t*)system_info;
	setsignature(info, info_signature);
	info->version = 1;
	info->flags = 0;
	info->unused = 0;
	info->heap_1_size = _heap_size;
	info->heap_2_size = 0;
	info->heap_3_size = 0;
	info->heap_4_size = 0;
	info->physical_memory = physical_memory_l;
	info->cpu_speed = cpu_speed;
	info->cpu_id_type = cpu_id_type;
	info->cpu_id_family = cpu_id_family;
	info->cpu_id_model = cpu_id_model;
	info->cpu_id_stepping = cpu_id_stepping;
	info->cpu_id_longmode = cpu_id_longmode;
	
	memset(info->cpu_id_name, 0, 32);
	memset(info->cpu_id_str, 0, 64);
	memset(info->cpu_brand_str, 0, 256);
	memset(info->reserved, 0, 104);
	strcpy(info->cpu_id_name, cpu_id_name);
	strcpy(info->cpu_id_str, cpu_id_str);
	strcpy(info->cpu_brand_str, cpu_brand_str);
	
	strcpy(info->reserved, "");	
}

void loadvars(void)
{
	char vars_signature[] = "VARS";
	char enum_signature[] = "ENUM";
	system_variables_info = (unsigned char *)0x808000;
	system_variables_enum_info = (unsigned char *)0x818000;
	sys_vars_info = (sys_vars_info_t *)system_variables_info;
	sys_enum_info = (sys_enum_info_t *)system_variables_enum_info;
	if (sys_vars_info->signature[0] == 0)
	{
		setsignature(sys_vars_info, vars_signature);
		sys_vars_info->version = 1;
	}
	if (sys_enum_info->signature[0] == 0)
	{
		setsignature(sys_enum_info, enum_signature);
		sys_enum_info->version = 1;
	}
	sys_vars_loaded = 1;
	sys_enum_loaded = 1;	
}

void enable_interrupt(void)
{
	//if (usermode) return;
	__asm__ ("sti");
}

void disable_interrupt(void)
{
	//if (usermode) return;
	__asm__ ("cli");
}

int main(void)
{
	enable_interrupt();
	init_heap();
	loadsysteminfo();
	loadvars();
	return 0;
}
