#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>	/* For O_RDWR */
#include <sys/mman.h>
#include <stdint.h>

struct constellation {
	float p;
	int32_t s1;
	int32_t s2;
	int32_t numstars;
	int32_t staridx;
	int32_t last_l;
	int32_t last_m;
	int32_t last_h;
	int32_t mmi_l;
	int32_t mmi_h;
};

int *map;
struct constellation *constptr;
int *star_ids;

void  add_entry(int mapidx,int curr_const) {
	int *staridx;
	for (staridx=&map[mapidx];*staridx!=-1;){
		if (mapidx==constptr[*staridx].mmi_l) {
			if (staridx==&(constptr[curr_const].last_l)) return; /* constelation has already been added */
			staridx=&(constptr[*staridx].last_l);
		} else if (mapidx==constptr[*staridx].mmi_h) {
			if (staridx==&(constptr[curr_const].last_h)) return; /* constelation has already been added */
			staridx=&(constptr[*staridx].last_h);
		}else {
			if (staridx==&(constptr[curr_const].last_m)) return; /* constelation has already been added */
			staridx=&(constptr[*staridx].last_m);
		}
	}
	*staridx=curr_const;
}

int main (int argc, char** argv) {
	//load config
	FILE *stream;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	stream = fopen("calibration/calibration.txt", "r");
	if (stream == NULL) exit(EXIT_FAILURE);
	while ((read = getline(&line, &len, stream)) != -1) {
		//Don't listen to valgrind. This is fine. Everything is fine.
		putenv(strcpy((char *)malloc(sizeof(char) * len),line));
	}
	fclose(stream);
	
	stream = fopen("calibration/dbsize.txt", "r");
	if (stream == NULL) exit(EXIT_FAILURE);
	while ((read = getline(&line, &len, stream)) != -1) {
		//Don't listen to valgrind. This is fine. Everything is fine.
		putenv(strcpy((char *)malloc(sizeof(char) * len),line));
	}
	fclose(stream);
	
	//constline
	stream=fopen("calibration/constellations.txt", "r");
	
	int PARAM=atoi(getenv("PARAM"));
	int NUMCONST=atoi(getenv("NUMCONST"));
	int STARTABLE=atoi(getenv("STARTABLE"));
	float ARC_ERR=atof(getenv("ARC_ERR"));
	int mapsize=PARAM;
	int s_offset=mapsize*sizeof(int)+ NUMCONST*sizeof(struct constellation);
	size_t dbsize = s_offset + STARTABLE*sizeof(int);
	
	/* Open a file for writing.
	 *  - Creating the file if it doesn't exist.
	 *  - Truncating it to 0 size if it already exists. (not really needed)
	 *
	 * Note: "O_WRONLY" mode is not sufficient when mmaping.
	 */

	const char *filepath = "beastdb.bin";
	int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
	if (fd == -1) {
		perror("Error opening file for writing");
		exit(EXIT_FAILURE);
	}

	// Stretch the file size to the size of the (mmapped) array of char


	if (lseek(fd, dbsize-1, SEEK_SET) == -1) {
		close(fd);
		perror("Error calling lseek() to 'stretch' the file");
		exit(EXIT_FAILURE);
	}
	
	if (write(fd, "", 1) == -1) {
		close(fd);
		perror("Error writing last byte of the file");
		exit(EXIT_FAILURE);
	}
	// Now the file is ready to be mmapped.

	map = (int*)mmap(0, dbsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
	{
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
	}
	constptr=(struct constellation*)(&map[mapsize]);
	star_ids=(int*)(&map[s_offset/sizeof(int)]);
	memset(map, -1, dbsize);
	int mmi_l,mmi_m,mmi_h;
	int curr_star=0;
	for (int curr_const=0;curr_const<NUMCONST;curr_const++){
		if ((read = getline(&line, &len, stream)) != -1) {
			constptr[curr_const].p=atof(strtok(line," "));
			constptr[curr_const].s1=atoi(strtok(NULL," "));
			constptr[curr_const].s2=atoi(strtok(NULL," "));
			constptr[curr_const].numstars=atoi(strtok(NULL," "));
			constptr[curr_const].staridx=curr_star;
			mmi_l=(int)(constptr[curr_const].p/ARC_ERR-1)%mapsize;
			mmi_m=(int)(constptr[curr_const].p/ARC_ERR)%mapsize;
			mmi_h=(int)(constptr[curr_const].p/ARC_ERR+1)%mapsize;
			if (mmi_l<0) mmi_l+=mapsize;
			if (mmi_m<0) mmi_m+=mapsize;
			if (mmi_h<0) mmi_h+=mapsize;
			constptr[curr_const].mmi_l=mmi_l;
			constptr[curr_const].mmi_h=mmi_h;
				
			if ((read = getline(&line, &len, stream)) != -1) {
				for (int i=0;i<constptr[curr_const].numstars;i++) {
					if (i==0) star_ids[curr_star]=atoi(strtok(line," "));
					else star_ids[curr_star]=atoi(strtok(NULL," "));
					curr_star++;
				}
				//add entry to constellation table
				add_entry(mmi_l,curr_const);
				add_entry(mmi_m,curr_const);
				add_entry(mmi_h,curr_const);
			}
		}
	}
	
	// Write it now to disk
	if (msync(map, dbsize, MS_SYNC) == -1) {
		perror("Could not sync the file to disk");
	}
	
	// Don't forget to free the mmapped memory
	if (munmap(map, dbsize) == -1) {
		close(fd);
		perror("Error un-mmapping the file");
		exit(EXIT_FAILURE);
	}
	// Un-mmaping doesn't close the file, so we still need to do that.
	close(fd);
	fclose(stream);
}
