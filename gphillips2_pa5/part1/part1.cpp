#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <vector>



/******************************************************
 * Declarations
 ******************************************************/
// #Define'd sizes
#define PAGE_SIZE 256
size_t ELEMENT_SIZE = 1;
size_t NUMBER_ELEMENTS = 256;
int frame_num = 0;
char *fname;
char *addressFile;
FILE *of;

// Make the TLB array
// Need pages associated with frames (could be 2D array, or C++ list, etc.)
int TLB[16][2];

// Make the Page Table
// Again, need pages associated with frames (could be 2D array, or C++ list, etc.)
int PAGE_TABLE[256][2];

// Make the memory
// Memory array (easiest to have a 2D array of size x frame_size)
char PHYSICAL_MEMORY[256][256];

/******************************************************
 * Function Declarations
 ******************************************************/

/***********************************************************
 * Function: get_page_and_offset - get the page and offset from the logical address
 * Parameters: logical_address
 *   page_num - where to store the page number
 *   offset - where to store the offset
 * Return Value: none
 ***********************************************************/
void get_page_and_offset(int logical_address, int *page_num, int *offset);

/***********************************************************
 * Function: get_frame_TLB - tries to find the frame number in the TLB
 * Parameters: page_num
 * Return Value: the frame number, else NOT_FOUND if not found
 ***********************************************************/
int get_frame_TLB(int page_num);

/***********************************************************
 * Function: get_available_frame - get a valid frame
 * Parameters: none
 * Return Value: frame number
 ***********************************************************/
int get_available_frame();

/***********************************************************
 * Function: get_frame_pagetable - tries to find the frame in the page table
 * Parameters: page_num
 * Return Value: page number, else NOT_FOUND if not found (page fault)
 ***********************************************************/
int get_frame_pagetable(int page_num);

/***********************************************************
 * Function: backing_store_to_memory - finds the page in the backing store and
 *   puts it in memory
 * Parameters: page_num - the page number (used to find the page)
 *   frame_num - the frame number for storing in physical memory
 * Return Value: none
 ***********************************************************/
void backing_store_to_memory(int page_num, int frame_num, const char *fname);

/***********************************************************
 * Function: update_page_table - update the page table with frame info
 * Parameters: page_num, frame_num
 * Return Value: none
 ***********************************************************/
void update_page_table(int page_num, int frame_num);

/***********************************************************
 * Function: update_TLB - update TLB (FIFO)
 * Parameters: page_num, frame_num
 * Return Value: none
 ***********************************************************/
void update_TLB(int page_num, int frame_num);


/******************************************************
 * Assumptions:
 *   If you want your solution to match follow these assumptions
 *   1. In Part 1 it is assumed memory is large enough to accommodate
 *      all frames -> no need for frame replacement
 *   2. Part 1 solution uses FIFO for TLB update
 *   3. In the solution binaries it is assumed a starting point at frame 0,
 *      subsequently, assign frames sequentially
 *   4. In Part 2 you should use 128 frames in physical memory
 ******************************************************/

int main(int argc, char * argv[]) {
		// argument processing
        fname = argv[1];
        addressFile = argv[2];

		// For Part2: read in whether this is FIFO or LRU strategy

		// initialization
		int *page_num = new int;
		int *offset = new int;
        int frame;
        int value;
        int count = 0;
        int physicalAddress;
        int logicalAddress;
        int pageTableHits;
        int TBLHits = 0;
        int pageFaults = 0;
        float PFR;
        float TLBHR;

        //Initalize the pagetables with non-zero, negative values
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 2; j++) {
                PAGE_TABLE[i][j] = -1;
            }
        }

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 2; j++) {
                TLB[i][j] = -1;
            }
        }

        of = fopen ("correct.txt","w");
        
        // read addresses.txt
        std::vector <std::string> addresses;
        std::string line;    
        std::ifstream fin(addressFile);
        while(getline(fin,line)){
            addresses.push_back(line);
        }

		while( count < addresses.size()) {
				// pull addresses out of the file
                logicalAddress = stoi(addresses[count]);
				// Step 0:
				// get page number and offset
				get_page_and_offset(logicalAddress,page_num,offset);
                //std::cout << "The page number is: " << *page_num <<std::endl;
                //std::cout << "The offset is: " << *offset <<std::endl;

				// need to get the physical address (frame + offset):
				// Step 1: check in TLB for frame
                frame = get_frame_TLB(*page_num);
                //std::cout << "The frame from TLB is: " << frame << std::endl;
                if (frame == -1) {
				//     Step 2: not in TLB, look in page table
                    frame = get_frame_pagetable(*page_num);
                    //std::cout << "The frame from page table is: " << frame << std::endl;
				    if (frame == -1) {
                        //std::cout << "PAGE FAULT" << std::endl;
				//      PAGE_FAULT!
                        pageFaults++;
				//      Step 3:
				        backing_store_to_memory(*page_num, frame_num, fname);
				//      Step 4:
                        update_page_table(*page_num, frame_num);
                        frame = frame_num;
                        frame_num++;
                    }
                    else {
                        pageTableHits++;
                    }
                    //Step 5: (always) update TLB when we find the frame  
				    //update TLB (updateTLB())
                    update_TLB(*page_num,frame);
                }

                else {
                    TBLHits++;
                }
				//   Step 6: read val from physical memory
                physicalAddress = (frame << 8) | *offset;
                
                value = PHYSICAL_MEMORY[frame][*offset];
                count++;
		        // output useful information for grading purposes
                fprintf(of, "Virtual address: %d Physical address: %d Value: %d\n",logicalAddress,physicalAddress,value);
                //std::cout << "Virtual Address: " << logicalAddress << " Physical address: " << physicalAddress << " Value: " << value << std::endl;
		}
        // std::cout << frame << " " << *offset << std::endl;
        // std::cout << "addresses " << count << std::endl;
        // std::cout << "Page Faults: " << pageFaults << std::endl;
        // std::cout << "TBL Hits: " << TBLHits << std::endl;
        // std::cout << "Page table Hits: " << pageTableHits << std::endl;
        PFR = (float) pageFaults / (float) count;
        TLBHR = (float) TBLHits / (float) count;
        fprintf(of,"Number of Translated Addresses = %d\n", count);
        fprintf(of,"Page Faults = %d\n", pageFaults);
        fprintf(of,"Page Fault Rate = %.3f\n", PFR);
        fprintf(of,"TLB Hits = %d\n",TBLHits);
        fprintf(of,"TLB Hit Rate = %.3f\n",TLBHR);
        fclose(of);

}

void get_page_and_offset(int logical_address, int *page_num, int *offset) {
	unsigned tempLow = logical_address & 0xFF;
	unsigned tempHigh = (logical_address >> 8);
	*page_num = tempHigh;
	*offset = tempLow;
}

void backing_store_to_memory(int page_num, int frame_num, const char *fname) {
    //open the file
    FILE *fid;
    fid = fopen (fname, "r");
    char temp[256];
    //we are looking for the byte that starts at some specific page number, the starting byte of
    //our frame will be page_num*page size
    long int offset = page_num * PAGE_SIZE;
    fseek(fid, offset, SEEK_SET);
    fread( temp, ELEMENT_SIZE, NUMBER_ELEMENTS, fid);
    fclose(fid);
    //store the page from BACKING_STORE into the physical memory
    for (int i = 0; i < 256; i++)
        PHYSICAL_MEMORY[frame_num][i] = temp[i];
}

void update_page_table(int page_num, int frame_num) {
    //First in First out
    for (int i = 255; i >= 0; i--) {
        PAGE_TABLE[i][0] = PAGE_TABLE[i-1][0];
        PAGE_TABLE[i][1] = PAGE_TABLE[i-1][1];
    }
    PAGE_TABLE[0][0] = page_num;
    PAGE_TABLE[0][1] = frame_num;
}

void update_TLB(int page_num, int frame_num) {
    //First in First out
    for (int i = 15; i >= 0; i--) {
        TLB[i][0] = TLB[i-1][0];
        TLB[i][1] = TLB[i-1][1];
    }
    TLB[0][0] = page_num;
    TLB[0][1] = frame_num;
}

int get_frame_TLB(int page_num) {
    for (int i = 0; i < 16; i++) {
        if (TLB[i][0]== page_num) {
            return TLB[i][1];
        }
    }
    return -1;
}

int get_frame_pagetable(int page_num) {
    for (int i = 0; i < 256; i++) {
        if (PAGE_TABLE[i][0] == page_num) {
            return PAGE_TABLE[i][1];
        }
    }
    return -1;
}