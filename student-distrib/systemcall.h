#ifndef _systemcall_H
#ifndef _systemcall_H

int32_t halt (uint_8 status);
int32_t execute (const uint8_t * command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* fileame);
int32_t close *(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t bytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_hadler(int32_t sigunum, void* handler_address);
int32_t sigretur(void);

#endif
