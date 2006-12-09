

#ifndef _U_UUID_H_
#define _U_UUID_H_


#ifdef __cplusplus
extern "C" {
#endif

#define MAC_LEN 6
#define SIZE_OF_UUID_STRING     37

int 
generate_uuid ( char* buf, 
                int size, 
                int no_prefix);

#ifdef __cplusplus
}
#endif

#endif /* !_U_UUID_H_ */ 
