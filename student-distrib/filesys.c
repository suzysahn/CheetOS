#include "filesys.h"
#include "syscalls.h"


static uint8_t * fs_ptr;
static file_t blank;

/* OPERATION TABLE */
static int32_t terminal_ops[4] = { (int32_t) &terminal_open, (int32_t) &terminal_read, (int32_t) &terminal_write, (int32_t) &terminal_close}; // open, read, write, close
static int32_t file_ops[4] = { (int32_t) &file_open, (int32_t) &file_read, (int32_t) &file_write, (int32_t) &file_close}; // open, read, write, close
static int32_t directory_ops[4] = { (int32_t) &directory_open, (int32_t) &directory_read, (int32_t) &directory_write, (int32_t) &directory_close}; // open, read, write, close
static int32_t rtc_ops[4] = { (int32_t) &rtc_open, (int32_t) &rtc_read, (int32_t) &rtc_write, (int32_t) &rtc_close}; // open, read, write, close


/* file_open
 * DESCRIPTION: opens the file by filename
 * INPUTS: pointer to the filename
 * OUTPUTS: int fd on success, -1 on failure
 * RETURN VALUE: int fd on success, -1 on failure
 * SIDE EFFECTS: using read dentry by name
 */
int32_t file_open(const uint8_t* filename)
{
  file_t * file_array = getCurrentProcessPCB()->file_array;
  dentry_t new_dirent;
  int32_t idx;

  // see if file exists, and check for the file name 
  if (filename == NULL) return -1;
  if (strlen((int8_t*)filename) >= FILENAME_LEN) return -1;
  if (read_dentry_by_name(filename, &new_dirent) == -1) return -1;

  // next aval file name
  for (idx = 2; idx < NUM_MAX_OPEN_FILES; idx++) 
  {
    if (file_array[idx].flags == FILE_AVAIL) /* check free space is file array */
      break;
  }

  /* if there are already too many opened files, return -1 */
  if (idx >= NUM_MAX_OPEN_FILES) return -1;

  if (new_dirent.filetype == 2) /* FILE */
  {
    file_array[idx].file_ops_table_ptr = (int32_t) file_ops;
    file_array[idx].inode_num = new_dirent.inode_num;
  }
  else if (new_dirent.filetype == 1) /* DIRECTORY */
  {
    file_array[idx].file_ops_table_ptr = (int32_t) directory_ops;
    file_array[idx].inode_num = new_dirent.inode_num;
  }
  else if (new_dirent.filetype == 0) /* RTC */
  {
    file_array[idx].file_ops_table_ptr = (int32_t) rtc_ops;
    file_array[idx].inode_num = new_dirent.inode_num;
    rtc_open(idx);
    return idx;
  }
  else 
  {
    return -1;
  }

  /* regardless of file */
  file_array[idx].file_position = 0;
  file_array[idx].flags = FILE_OCCUP;

  return idx;
}

/* file_close
 * DESCRIPTION: closes the file 
 * INPUTS: the index of fd
 * OUTPUTS: 0 if success, -1 if fail
 * RETURN VALUE: 0 if success, -1 if fail
 * SIDE EFFECTS: none
 */
int32_t file_close(int32_t fd)
{
  file_t * file_array = getCurrentProcessPCB()->file_array;
  if (fd >= NUM_MAX_OPEN_FILES || fd < 0) /* out of bounds */
    return -1;
  else if (file_array[fd].flags == FILE_AVAIL) /*try to open closed files*/
    return -1;
  else
    file_array[fd] = blank;
  return 0;
}

/* file_read
 * DESCRIPTION: reads the count data into the buffer
 * INPUTS: index of file descriptor, the buffer, and the number of bytes
 * OUTPUTS: nbytes if successful
 * RETURN VALUE: nybytes if successful
 * SIDE EFFECTS: none
 */
int32_t file_read(int32_t fd, int8_t* buf, int32_t nbytes)
{
  file_t * file_array = getCurrentProcessPCB()->file_array;
  if (fd == 1 || fd == 0) /* stdin and out, don't do anything return -1*/
    return -1;

  uint32_t inode = file_array[fd].inode_num;
  uint32_t os = file_array[fd].file_position;

  /*return nbytes*/
  uint32_t read = read_data(inode, os, (uint8_t *) buf, (uint32_t) nbytes);
  file_array[fd].file_position += read;
  return read;
}

/* file_write
 * DESCRIPTION: writes to the file, except it's a read only file system so nothing
 * INPUTS: fd, pointer to buf, nbytes
 * OUTPUTS: -1
 * RETURN VALUE: -1
 * SIDE EFFECTS: none
 */
int32_t file_write(int32_t fd, const int8_t* buf, int32_t nbytes)
{
  return -1; 
}

/*
 * flength
 * DESCRIPTION: returns file length
 * INPUTS:  inode
 * OUTPUTS: file length
 * RETURN VALUE: file length
 */
int32_t flength(uint32_t inode){
    inode_block_t* inode_bs = (inode_block_t*)(fs_ptr + 4096 * (inode + 1));
    return inode_bs->length;
}


/* directory_open
 * DESCRIPTION: opens a directory file
 * INPUTS: pointer to 8 bit filename
 * OUTPUTS:
 * RETURN VALUE: int fd on success, int -1 on failure
 * SIDE EFFECTS: uses read_dentry_by_name
 */
int32_t directory_open(const uint8_t* filename)
{
  dentry_t new_dirent;

  /* if not found then return -1 */
  if (read_dentry_by_name(filename, &new_dirent) == -1)
      return -1;
  if (new_dirent.filetype != 1) // not a directory
    return -1;
  /* open file */
  return file_open(filename);
}

/* directory_close
 * DESCRIPTION: close the file
 * INPUTS: fd
 * OUTPUTS: 0 success, -1 fail
 * RETURN VALUE: same as output
 * SIDE EFFECTS: none
 */
int32_t directory_close(int32_t fd)
{
  return file_close(fd);
}

/* directory_read
 * DESCRIPTION: reads files by name
 * INPUTS: int32_t fd, int8_t* buf, int32_t nbytes
 * OUTPUTS: nbytes
 * RETURN VALUE: nbytes
 * SIDE EFFECTS: none
 */
int32_t directory_read(int32_t fd, int8_t* buf, int32_t nbytes)
{
  /* get current pcb's file array */
  file_t * file_array = getCurrentProcessPCB()->file_array;
  dentry_t new_dirent;

  int32_t i = 0;
  if (read_dentry_by_index(file_array[fd].file_position, &new_dirent) == -1) {
     return -1;
  }

  /* next file, next */
  file_array[fd].file_position++;
  while (new_dirent.filename[i] != '\0' && i < 32) {
      buf[i] = new_dirent.filename[i];
      i++;
  }

  /* fill buffer with empty spaces, return i*/
  buf[i] = '\0';
  return i;
}

/* directory_write
 * DESCRIPTION: does nothing
 * INPUTS: int32_t fd, const int8_t* buf, int32_t nbytes
 * OUTPUTS: -1
 * RETURN VALUE: 0 success, -1 fail
 * SIDE EFFECTS: none
 */
int32_t directory_write(int32_t fd, const int8_t* buf, int32_t nbytes)
{
  return -1; /*does nothing */
}

/* read_dentry_by_name
 * DESCRIPTION: reads directory entry by name
 * INPUTS: filename
 * OUTPUTS: directory entry 
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
  /* search bootblock thru filename */
  uint32_t index;

  if (fname == NULL){
    return -1;
  }

  /* empty string, return 0*/
  if(fname[0] == '\0')
    return -1;

  /* locate dir entry */
  for (index = 0; index < NUM_FILES; index++){

    /* if no match go to nexgt file */
    if (strncmp((int8_t*) fname, ((boot_block_t*) fs_ptr)->direntries[index].filename, FILENAME_LEN) == 0){
      read_dentry_by_index(index, dentry);
      return 0;
    }
  }
  return -1;
}

/* read_dentry_by_index
 * DESCRIPTION: reads directory entry by index
 * INPUTS: index, dentry
 * OUTPUTS: directory entry by index
 * RETURN VALUE: 0 on success, -1 fail
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
  /* read if index is invalid */
  if (index >= NUM_FILES || index < 0){
      return -1;
  }

  /* copy the directory entry */
  *dentry = ((boot_block_t*) fs_ptr)->direntries[index];

  return 0;
}

/* read_data
 * DESCRIPTION: read data
 * INPUTS: inode , offset, buffer, length
 * OUTPUTS: daata into buf
 * RETURN VALUE: nbytes
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
  /* local variables */
  int32_t count, index_in_inode, index_in_data_block, data_block_num, done;
  inode_block_t* inode_block;
  uint8_t* data_block;

  /* check in range */
  if (inode >= ((boot_block_t*) fs_ptr)->inode_count){
    return -1;
  }

// /* how many files open? */
  count = 0;

  /* get inode block */
  inode_block = (inode_block_t*) (fs_ptr + NUM_B_IN_FOUR_KB * (inode + 1));

  if (offset >= inode_block->length){
    return count;
  }

  /* calculate the index_in_inode */
  index_in_inode = offset / NUM_B_IN_FOUR_KB;
  index_in_data_block = offset % NUM_B_IN_FOUR_KB;
  done = 0;

  /* if not done reading into buf, get data block number, and get data block */
  while (done == 0){
    data_block_num = inode_block->data_block_num[index_in_inode];
    if (data_block_num >= ((boot_block_t*) fs_ptr)->data_count){
      return -1;
    }
    data_block = (uint8_t*) (fs_ptr + NUM_B_IN_FOUR_KB * (((boot_block_t*) fs_ptr)->inode_count + data_block_num + 1));

   /* read into buf */
    while (count < length && index_in_data_block < NUM_B_IN_FOUR_KB && offset < inode_block->length){
      buf[count] = data_block[index_in_data_block];
      index_in_data_block++;
      offset++;
      count++;
    }

    /* if maximum bytes reached then put flag back */
    if (count >= length || offset >= inode_block->length){
      done = 1;
    }
    index_in_data_block = 0;
    index_in_inode++;
  }
  return count;
}

/* filesys_init
 * DESCRIPTION: initializes the filesystem
 * INPUTS: ptr
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECTS: empty 
 */
void filesys_init(uint32_t ptr)
{
  fs_ptr = (uint8_t *) ptr;

  /* initialize the ptrs */
  blank.file_ops_table_ptr = 0;
  blank.inode_num = 0;
  blank.file_position = 0;
  blank.flags = FILE_AVAIL;
}

/* init_file_array
 * DESCRIPTION: files array with blank entries
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 */
void init_file_array(file_t * file_array)
{
  uint32_t i;
  /* initialize file array */
  for (i = 0; i < NUM_MAX_OPEN_FILES; i++)
  {
    file_array[i] = blank;
  }

  // stdin
  file_array[0].file_ops_table_ptr = (int32_t) terminal_ops;
  file_array[0].inode_num = 0;
  file_array[0].file_position = 0;
  file_array[0].flags = FILE_OCCUP;

  // stdout
  file_array[1].file_ops_table_ptr = (int32_t) terminal_ops;
  file_array[1].inode_num = 0;
  file_array[1].file_position = 0;
  file_array[1].flags = FILE_OCCUP;
}
