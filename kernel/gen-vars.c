#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>



//#define MAX_ENUM 1024

#define MAX_ENUM 128
#define MAX_VARS 128


#pragma pack (push, 1)

typedef struct
{
	char name[128];
	char value[128];
} sys_vars_t;

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
	char name[MAX_ENUM][65];
	int value[MAX_ENUM];
} sys_enum_t;

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


sys_vars_t *sys_vars;
sys_vars_info_t *sys_vars_info;

sys_enum_t *sys_enum;
sys_enum_info_t *sys_enum_info;


int sys_is_enabled_enum(unsigned long enum_id);
void sys_enable_enum(unsigned long enum_id);
void sys_disable_enum(unsigned long enum_id);



int sys_max_vars = MAX_VARS;
char sys_vars_value[MAX_VARS][128];
char sys_vars_name[MAX_VARS][128];

int sys_max_enum;
int sys_enum_value[MAX_ENUM];
char sys_enum_name[MAX_ENUM][65];


#ifndef RAND_MAX
#define RAND_MAX 32767
#endif

unsigned long int rand_next = 1;

void fsrand(unsigned int seed)
{
    rand_next = seed;
}

int frand(void)
{
    int r;
    rand_next = ((rand_next * 1103515245) + 12345);
    r = ((unsigned int)(rand_next / 65536) % 32768);
    return r;
}

char *HexToStr(unsigned long val)
{
    char table[17] = {"0123456789abcdef"};
    char hex[8];
    char *r;
    int i=val;
    memset(hex, 0, 8);
    r = (char*)hex;
    char *s = r;
    char *rev = s;
    if (val == 0)
    {
        *s++ = '0';
    }    
    while(i > 0)
    {
        *s++ = table[(i % 16)];
        i /= 16;
    }
    *s-- = '\0';
    while(rev < s)
    {
        char ch = *rev;
        *rev++ = *s;
        *s-- = ch;
    }
    r = (char*)hex;
    return r;
}

unsigned long StrToHex(char* str)
{
    unsigned long hex,val;
    int n;
    hex=0;
    for(n=0;n<strlen(str);n++) 
    {
		if(str[n] >= '0' && str[n] <= '9')
		{
			val = (unsigned long)(str[n]-'0');
		}
		else if(str[n] >= 'a' && str[n] <= 'f')
		{
			val = ((unsigned long)(str[n]-'a')) + 0xA;
		}
		else if(str[n] >= 'A' && str[n] <= 'F')
		{
			val = ((unsigned long)(str[n]-'A')) + 0xA;
		}
		else
		{
			val = 0;
		}
		hex <<= 4;
		hex |= val;
    }
    return hex;
}

void uuidv4(char *str)
{
    char uuid[37];
    char base[37];
    strcpy(base, "10000000-1000-4000-8000-100000000000");
    int len = strlen(base);
    int slen=0;
    uuid[0] = '\0'; 
    for(int i=0;i<len;i++)
    {
        if ((base[i] >= '0') && (base[i] <= '9'))
        {
            char ch[2];
            ch[0] = base[i];
            ch[1] = '\0';
            unsigned char n = StrToHex(ch);
            unsigned char c = (n ^ frand() & (15 >> (n / 4)));
            if (slen == 12) c = ((base[slen+2] - '0') % 10);
            char hex[2];
            strcpy(hex, HexToStr(c));
            strcat(uuid, hex);
            if (slen == 7) strcat(uuid, "-");
            if (slen == 11) strcat(uuid, "-");
            if (slen == 15) strcat(uuid, "-");
            if (slen == 19) strcat(uuid, "-");
            slen++;
        }
    }
    slen += 4;
    uuid[slen] = '\0';     
    strcpy(str, uuid);
}

int uuidv4_validate(const char *str)
{
    int r=0, v=0, i=0;
    if (!str) return 0;
    if (strlen(str) != 36) return 0;
    while(*str)
    {
        if ((i == 8) && (*str != '-')) v--;
        if ((i == 13) && (*str != '-')) v--;
        if ((i == 18) && (*str != '-')) v--;
        if ((i == 23) && (*str != '-')) v--;
        if (((*str >= '0') && (*str <= '9')) || 
        ((*str >= 'a') && (*str <= 'f')) || 
        ((*str >= 'A') && (*str <= 'F')))
        {
            if (((i >= 0) && (i <= 7)) || 
                ((i >= 9) && (i <= 12)) || 
                ((i >= 14) && (i <= 17)) || 
                ((i >= 19) && (i <= 22)) || 
                ((i >= 24) && (i <= 36)))
            {
                if ((i == 14) && (*str != '4')) v--;
                if ((i == 19) && 
                ((*str != '8' && (*str != '9' && 
                (*str != 'a' && (*str != 'b')) && 
                (*str != 'A' && (*str != 'B')))))) v--;
                v++;
            }
        }
        i++;
        str++;
    }
    if (v == 32) r = 1;
    return r;
}


unsigned char checksum_str (char *str)
{
  unsigned char sum = 0;                                                                                                                                                                                           
  for (int i=128;i!=0;i--)
  {               
      sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *str++;
  }   
  return sum;
}

unsigned long get_sys_vars_checksum(void)
{
    unsigned long checksum=0;
	unsigned long vars_count=sys_vars_info->count;
    for(int i=0;i<vars_count;i++)
    {
        checksum += checksum_str(sys_vars[i].name);
        checksum += checksum_str(sys_vars[i].value);
        checksum += vars_count;
        checksum += vars_count >> i;
    }
    checksum <<= 4;
    checksum &= 0xffffffff;
	return checksum;
}

char * sys_get_var_name(unsigned long var_id)
{
  char *s;
  for (int i=0;i<sys_max_vars;i++)
  {
	  if (var_id == i)
	  {
		 if (strlen(sys_vars_name[i]) > 0)
		 {
			 s = sys_vars_name[i];
			 return s;
		 }
	  }
  }
  return NULL;
}

int sys_var_exists(char* var_name)
{
  int x = 0;
  for (int i=0;i<sys_max_vars;i++)
  {
    if (strcmp(sys_vars_name[i],var_name)==0) x = 1;
  }
  return x;
}

int sys_get_var(unsigned long var_id, char *var_value)
{
  if (var_id >= MAX_VARS) return 0;
  strcpy(var_value, sys_vars_value[var_id]);
  return 1;
}

int sys_get_var_index(char* var_name)
{
  for (int i=0;i<sys_max_vars;i++)
  {
    if (strcmp(sys_vars_name[i],var_name)==0) return i;
  }
  return 0;
}

unsigned long get_sys_enum_checksum(void)
{
    unsigned long checksum=0;
	unsigned long enum_count=sys_enum_info->count;
    for(int i=0;i<enum_count;i++)
    {
        checksum += checksum_str(sys_enum->name[i]);
        checksum += (sys_enum->value[i]) << 4;
        checksum += enum_count;
        checksum += enum_count >> i;
    }
    checksum <<= 4;
    checksum &= 0xffffffff;
	return checksum;
}

int sys_is_enum_exists(char* enum_name)
{
  int x = 0;
  for (int i=0;i<MAX_ENUM;i++)
  {
    if (strcmp(sys_enum_name[i],enum_name)==0) x = 1;
  }
  return x;
}

int sys_enum_exists(char* enum_name)
{
  int x = 0;
  for (int i=0;i<sys_max_enum;i++)
  {
    if (strcmp(sys_enum_name[i],enum_name)==0) x = 1;
  }
  return x;
}

char * sys_get_enum_name(unsigned long enum_id)
{
  char *s;
  for (int i=0;i<sys_max_enum;i++)
  {
	  if (enum_id == i)
	  {
		 if (strlen(sys_enum_name[i]) > 0)
		 {
			 s = sys_enum_name[i];
			 return s;
		 }
	  }
  }
  return NULL;
}

int sys_get_enum_value(char* enum_name)
{
  if (sys_enum_exists(enum_name))
  {
   for (int i=0;i<sys_max_enum;i++)
   {
     if (strcmp(sys_enum_name[i],enum_name)==0) return sys_enum_value[i];
   }
  }
  return 0;
}
int sys_is_enabled_enum(unsigned long enum_id)
{
  if (enum_id >= MAX_ENUM) return 0;
  bool IsEnabled = false;
  if (sys_enum_value[enum_id] == 0) IsEnabled = false;
  else if (sys_enum_value[enum_id] == 1) IsEnabled = true;
  return IsEnabled;
}
void sys_enable_enum(unsigned long enum_id)
{
  if (enum_id < MAX_ENUM) sys_enum_value[enum_id] = true;
}
void sys_disable_enum(unsigned long enum_id)
{
  if (enum_id < MAX_ENUM) sys_enum_value[enum_id] = false;
}

int sys_get_enum(unsigned long enum_id)
{
  if (enum_id >= MAX_ENUM) return 0;
  return sys_enum_value[enum_id];
}

int sys_get_exists(char* enum_name)
{
  int x = 0;
  for (int i=0;i<sys_max_enum;i++)
  {
    if (strcmp(sys_enum_name[i],enum_name)==0) x = 1;
  }
  return x;
}

static int readconfig(FILE *fp, const char *key, char *value, size_t len)
{
	char buf[1000], *k, *v, *eq;
	int x=0;

	if (len < 1) return 0;

	rewind(fp);

	while (1) {
		if (!fgets(buf, 1000, fp)) return 0;

		if (buf[0] == ';') continue;

		eq = strchr(buf, '=');
		if (!eq) continue;

		k = buf;
		v = eq+1;

		while (*k == ' ' || *k == '\t') k++;
		*(eq--) = 0;
		while ((*eq == ' ' || *eq == '\t') && eq>=k) *(eq--) = 0;

		if (strcasecmp(k, key)) continue;

		while (*v == ' ' || *k == '\t') v++;
		eq = v + strlen(v)-1;

		while ((*eq == ' ' || *eq == '\t' || *eq == '\r' || *eq == '\n') && eq>=v) *(eq--) = 0;

		value[--len] = 0;
		do value[x] = v[x]; while (v[x++] != 0 && len-- > 0);

		return x-1;
	}
}

static int getconfig(FILE *fp, char *key, size_t len)
{
	char buf[1000], *k, *v, *eq;
	int x=0;

	if (len < 1) return 0;

	while (1) {
		if (!fgets(buf, 1000, fp)) return 0;

		if (buf[0] == ';') continue;

		eq = strchr(buf, '=');
		if (!eq) continue;

		k = buf;
		v = eq+1;

		while (*k == ' ' || *k == '\t') k++;
		*(eq--) = 0;
		while ((*eq == ' ' || *eq == '\t') && eq>=k) *(eq--) = 0;

		while (*v == ' ' || *k == '\t') v++;
		eq = v + strlen(v)-1;

		while ((*eq == ' ' || *eq == '\t' || *eq == '\r' || *eq == '\n') && eq>=v) *(eq--) = 0;

		key[--len] = 0;
		do key[x] = k[x]; while (k[x++] != 0 && len-- > 0);

		return x-1;
	}
}


int loadenum_config(const char *fn)
{
	FILE *fp;
#define VL 256
	char val[VL];

	if ((fp = fopen(fn, "rt")) == NULL) return -1;

    printf("Loading enum.cfg\n");

	if (readconfig(fp, "max_enum", val, VL) > 0) {
		sys_max_enum = atol(val);
	}
	else return -8;
	
        if (sys_max_enum >= MAX_ENUM)
        {
          fclose(fp);
	  return -2;
        }
		
        for (int i=0;i<sys_max_enum;i++)
        {
          if (strlen(sys_enum_name[i]) > 0)
          {
            char enum_s[256];
            strcpy(enum_s,"");
            sprintf(enum_s,"sys_enum[\"%s\"]",sys_enum_name[i]);
   	    if (readconfig(fp, enum_s, val, VL) > 0) {

            if (strlen(val) > 0)
            {
                if (isdigit(val[0]))
                {
                     sys_enum_value[i] = atol(val);
                }
                else if (isalpha(val[0]))
                {
                     if (strcmp(sys_enum_name[i],val)==0)
                     {
                       if (sys_enum_exists(val)) sys_enum_value[i] = i;
                       else
                       {
                        fclose(fp);
                        return -4;
                       }
                     }
                     else
                     {
                        fclose(fp);
                        return -5;
                     }
                }
                else
                {
                  fclose(fp);
                  return -6;
                }
  	      }
              else
              {
                fclose(fp);
                return -7;
              }
            }
          }
          else
          {
             fclose(fp);
             return -3;
          }
        }
	fclose(fp);

	return 0;
}

int loadvars_config(const char *fn)
{
	FILE *fp;
#define VL 256
	char val[VL];
	char v2[VL];

	if ((fp = fopen(fn, "rt")) == NULL) return -1;

    printf("Loading vars.cfg\n");

	if (readconfig(fp, "max_vars", val, VL) > 0) {
		sys_max_vars = atol(val);
	}
	else return -8;
	
        if (sys_max_vars >= MAX_ENUM)
        {
          fclose(fp);
		  return -2;
        }
		
		
        for (int i=0;i<sys_max_vars;i++)
        {
            char var_s[256];
            char var_name[256];
            strcpy(var_s,"");
            strcpy(var_name,"");
			if (getconfig(fp, var_s, VL) > 0) 
			{
				sscanf(var_s, "sys_vars[\"%s\"]", var_name);
				if (strcmp(var_name+(strlen(var_name)-3), "\"]"))
				{
					memset(var_name+(strlen(var_name)-2), 0, 2);
				}
				strcpy(sys_vars_name[i], var_name);
			}
		}
		
		
        for (int i=0;i<sys_max_vars;i++)
        {
          if (strlen(sys_vars_name[i]) > 0)
          {
            char var_s[256];
            strcpy(var_s,"");
            sprintf(var_s,"sys_vars[\"%s\"]",sys_vars_name[i]);
			if (readconfig(fp, var_s, val, VL) > 0) 
			{
              if (strlen(val) > 0)
              {
                if (val[0] == '"')
                {					 
					 char var_value[256];
					 strncpy(var_value, val+1, strlen(val)-2);
					 var_value[strlen(val)-2] = '\0';
                     if (var_value[0] == '$')
                     {
					   char var_find[257];
					   int k = strlen(var_value);
					   int j=0;
					   int n=0;
					   while (j < k) 
					   {
						   if (var_value[j] == ':') 							   
						   {
							   k -= (strlen(var_value)-j+1);
							   n = 1;
						   }
						   j++;
					   }
					   strncpy(var_find, var_value+1, k);
					   var_find[k] = '\0';
                       if (sys_var_exists(var_find)) 
					   {
						   sys_get_var(sys_get_var_index(var_find), var_value);
						   strcpy(sys_vars_value[i], var_value);
						   if (n)
						   {
							   int w=strlen(val)-1;
							   int x=strlen(var_find)+2;
							   w -= x;
							   strncpy(v2, val+x, w);
							   v2[w] = '\0';
							   strcat(sys_vars_value[i], v2);
						   }
					   }
                       else
                       {
                        fclose(fp);
                        return -4;
                       }
                     }
                     else
                     {
                        strcpy(sys_vars_value[i], var_value);
                     }
                }
                else
                {
                  fclose(fp);
                  return -6;
                }
			  }
              else
              {
                fclose(fp);
                return -7;
              }
            }
          }
          else
          {
             fclose(fp);
             return -3;
          }
        }
	fclose(fp);

	return 0;
}

void InitSystem_ErrorMessage(const char* msg, ...)
{
	char msgBuf[1024];
	va_list va_alist;
        int nlc=0;

	if (!msg) return;

      	msgBuf[1024 - 1] = '\0';
	va_start(va_alist, msg);
	vsprintf(msgBuf, msg, va_alist);
	va_end(va_alist);

	printf("%s", msgBuf);
}

void InitSystem_WarningMessage(const char* msg, ...)
{
	char msgBuf[1024];
	va_list va_alist;
        int nlc=0;

	if (!msg) return;

      	msgBuf[1024 - 1] = '\0';
	va_start(va_alist, msg);
	vsprintf(msgBuf, msg, va_alist);
	va_end(va_alist);

    printf("%s", msgBuf);
}

long loadenum_names(void)
{
    char buffer[1024], *p, *name, *number, *endptr;
    long num, syms=0, line=0, a, comment=0;
    FILE *fp;
	int errors = 0, warnings = 0;

    fp = fopen("ENUM.H","r");
    if (!fp)
    {
        if ((fp = fopen("enum.h","r")) == NULL)
        {
            InitSystem_ErrorMessage("Could not find the enumeration file!\nFailed to open ENUM.H\n");
            return -1;
        }
    }

    memset(sys_enum_name,0,sizeof(sys_enum_name));

    printf("Loading enum.h\n");

    while (fgets(buffer, 1024, fp))
    {
        a = strlen(buffer);
        if (a >= 1)
        {
            if (a > 1)
                if (buffer[a-2] == '\r') buffer[a-2] = 0;
            if (buffer[a-1] == '\n') buffer[a-1] = 0;
        }

        p = buffer;
        line++;
        while (*p == 32) p++;
        if (*p == 0) continue;	// blank line

        if (*p == '#' && !comment)
        {
            p++;
            while (*p == 32) p++;
            if (*p == 0) continue;	// null directive

            if (!strncmp(p, "define ", 7))
            {
                // #define_...
                p += 7;
                while (*p == 32) p++;
                if (*p == 0)
                {
                    InitSystem_ErrorMessage("Error: Malformed #define at line %d\nMake sure the file 'enum.h' is configured correctly.\n", line-1);
					errors++;
                    continue;
                }

                name = p;
                while (*p != 32 && *p != 0) p++;
                if (*p == 32)
                {
                    *(p++) = 0;
                    while (*p == 32) p++;
                    if (*p == 0)  	// #define_ENUM_NAME with no number
                    {
                        InitSystem_ErrorMessage("Error: No number given for name \"%s\" (line %d)\nMake sure the file 'enum.h' is configured correctly.\n", name, line-1);
						errors++;
                        continue;
                    }

                    number = p;
                    while (*p != 0) p++;
                    if (*p != 0) *p = 0;

                    // add to list
                    num = strtol(number, &endptr, 10);
                    if (*endptr != 0)
                    {
                        p = endptr;
                        goto badline;
                    }
                    //printf("Grokked \"%s\" -> \"%d\"\n", name, num);
                    if (num < 0 || num >= MAX_ENUM)
                    {
                        InitSystem_ErrorMessage("Error: Constant %d for enumeration name \"%s\" out of range (line %d)\nMake sure the file 'enum.h' is configured correctly.\n", num, name, line-1);
						errors++;
                        continue;
                    }

                    if (strlen(name) > 64)
					{
                        InitSystem_WarningMessage("Warning: Name \"%s\" longer than 64 characters (line %d). Truncating.\nMake sure the file 'enum.h' is configured correctly.\n", name, line-1);
						warnings++;
					}

                    if (sys_is_enum_exists(name))  
					{
						InitSystem_WarningMessage("Warning: Name \"%s\" duplicated (line %d).\nThis can cause conflict with the existing enumeration. Truncating.\nMake sure the file 'enum.h' is configured correctly.\n", name, line-1);
						warnings++;
					}

                    strncpy(sys_enum_name[num], name, 64);
                    sys_enum_name[num][64] = 0;

                    syms++;

                    continue;

                }
                else  	// #define_ENUM_NAME with no number
                {
                    InitSystem_ErrorMessage("Error: No number given for enumeration name \"%s\" (line %d)\nMake sure the file 'enum.h' is configured correctly.\n", name, line-1);
					errors++;
                    continue;
                }
            }
            else goto badline;
        }
        else if (*p == '/')
        {
            if (*(p+1) == '*') {comment++; continue;}
            if (*(p+1) == '/') continue;	// comment
        }
        else if (*p == '*' && p[1] == '/')
        {
            comment--; continue;
        }
        else if (comment)continue;
		badline:
		{
			InitSystem_ErrorMessage("Error: Invalid statement found at character %d on line %d\nMake sure the file 'enum.h' is configured correctly.\n", (p-buffer), line-1);
			errors++;
		}
    }
    if (errors == 0) 
	{
		printf("Read %d lines, loaded %d enum names.\n", (int)line, (int)syms);
	}

    fclose(fp);
	
    return -errors;
}


char * itoa( int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if ( value < 0 && base == 10 )
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

void loadvars(void)
{
	char vars_uuid[38];
	char enum_uuid[38];
	uuidv4(vars_uuid);
	uuidv4(enum_uuid);
	sys_vars = (sys_vars_t *)malloc(sys_max_vars*sizeof(sys_vars_t));
	sys_vars_info = (sys_vars_info_t *)malloc(sizeof(sys_vars_info_t));
	strcpy(sys_vars_info->signature, "VARS");
	sys_vars_info->version = 1;
	sys_vars_info->count = sys_max_vars;
	strcpy(sys_vars_info->id, vars_uuid);
	memset(sys_vars, 0, (sys_max_vars*sizeof(sys_vars_t)));
	
	sys_enum = (sys_enum_t *)malloc(sizeof(sys_enum_t));
	sys_enum_info = (sys_enum_info_t *)malloc(sizeof(sys_enum_info_t));
	strcpy(sys_enum_info->signature, "ENUM");
	sys_enum_info->version = 1;
	sys_enum_info->count = sys_max_enum;
	strcpy(sys_enum_info->id, enum_uuid);
	memset(sys_enum, 0, (sizeof(sys_vars_t)));
	
	
}

void unloadvars(void)
{
	free(sys_vars);
	free(sys_vars_info);
	free(sys_enum);
	free(sys_enum_info);
}


void init_variables(void)
{
	
	int i=0;
	char var_name[128];
	char var_value[128];
	char enum_name[65];
	int enum_value;
	for(i=0;i<sys_vars_info->count;i++)
	{
		if (sys_get_var_name(i) != NULL)
		{
			strcpy(var_name, sys_get_var_name(i));
			strcpy(sys_vars[i].name, var_name);
			if (sys_get_var(i, var_value))
			{
				strcpy(sys_vars[i].value, var_value);
			} else
			{
				strcpy(sys_vars[i].value, "");
			}
		}
		else
		{
			memset(sys_vars[i].name, 0, 128);
			memset(sys_vars[i].value, 0, 128);
		}
		if (sys_vars[i].name != NULL)
		{
			printf("sys_vars[\"%s\"] = \"%s\"\n", sys_vars[i].name, sys_vars[i].value);
		}
	}	
	for(i=0;i<sys_enum_info->count;i++)
	{
		if (sys_get_enum_name(i) != NULL)
		{
			strcpy(enum_name, sys_get_enum_name(i));
			strcpy(sys_enum->name[i], enum_name);
			enum_value = sys_get_enum(i);
			sys_enum->value[i] = enum_value;
		}
		else
		{
			memset(sys_enum->name[i], 0, 65);
			memset(&sys_enum->value[i], 0, 4);
		}
		if (sys_vars[i].name != NULL)
		{
			printf("sys_enum[\"%s\"] = %d\n", sys_enum->name[i], sys_enum->value[i]);
		}
	}	
	sys_vars_info->checksum = get_sys_vars_checksum();	
	sys_enum_info->checksum = get_sys_enum_checksum();
}

int generatefiles(void)
{
	FILE *vars_fp;
	FILE *vars_info_fp;
	FILE *enum_fp;
	FILE *enum_info_fp;
	int vars_count = sys_vars_info->count;
	int enum_count = sys_enum_info->count;
	
	vars_fp = fopen("vars.bin", "wb");
	if (!vars_fp) return -1;
	fwrite(sys_vars, vars_count*sizeof(sys_vars_t), 1, vars_fp);
	fclose(vars_fp);	
	
	vars_info_fp = fopen("vars-info.bin", "wb");
	if (!vars_info_fp) return -2;
	fwrite(sys_vars_info, sizeof(sys_vars_info_t), 1, vars_info_fp);	
	fclose(vars_info_fp);
	
	enum_fp = fopen("enum.bin", "wb");
	if (!enum_fp) return -3;
	fwrite(sys_enum, sizeof(sys_enum_t), 1, enum_fp);
	fclose(enum_fp);	
	
	enum_info_fp = fopen("enum-info.bin", "wb");
	if (!enum_info_fp) return -4;
	fwrite(sys_enum_info, sizeof(sys_enum_info_t), 1, enum_info_fp);	
	fclose(enum_info_fp);
	
	return 0;
}


int main(int argc, char *argv[])
{
	int r_val=1;
	fsrand(time(NULL));
	if (loadenum_names() == 0)
	{
		if (loadenum_config("enum.cfg") == 0)
		{
			printf("Loaded %d enum names.\n", (int)sys_max_enum);	
			if (loadvars_config("vars.cfg") == 0)
			{
				printf("Loaded %d variables names.\n", (int)sys_max_vars);	
				loadvars();
				init_variables();
				if (generatefiles() == 0)
				{
					printf("Generated files successfully.\n");
					r_val = 0;
				}
				else
				{
					printf("Error: Cannot generate variables files.\n");
				}
				unloadvars();
			}
			else
			{
				printf("Error\n");
			}
		}
		else
		{
			printf("Error\n");
		}
	}
	return r_val;
}