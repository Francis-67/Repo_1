/***************************************************************************
 *   
 *   
 *   
 ***************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ldv.h>
#include "OpenLDVdefinitions.h"


#define LON_DATA_CODE_SZ	1
//#define U20_SERIAL			"4DB5BC04"ldv_msg[13] -> 00

#define U20_SERIAL			"4F0BEE05"   //"A1DB9642"


typedef enum {FALSE, TRUE} boolean;

typedef enum {
	LON_ADDR_MODE_IMPLICIT,
	LON_ADDR_MODE_EXPLICIT
} lon_addr_mode_t;


// Function prototypes
int lon_write_msg (LDV_HANDLE ldv_handle, unsigned int queue, unsigned char service, 
					unsigned char code, unsigned char data[], int data_size,
					unsigned char addr_mode, unsigned char addr_data[]);
int lon_read_msgs (LDV_HANDLE ldv_handle, ExpAppBuffer* ldv_msg);
bool lon_close (LDV_HANDLE ldv_handle);
bool display_packet(unsigned char *ptr, int msg_len);
bool reset_node(LDV_HANDLE ldv_handle);
bool assemble_packet(ExpAppBuffer* ldv_msg, unsigned int queue, unsigned char service, 
                    unsigned char function_code, unsigned char data[], int data_size,
					unsigned char addr_mode, unsigned char addr_data[]); 
bool update_nv(LDV_HANDLE ldv_handle, int index, unsigned char data[], int size);
bool retrieve_nv(LDV_HANDLE ldv_handle, int index, unsigned char data[], int size);
bool clear_network_buffer(LDV_HANDLE ldv_handle);
bool manual_service_pin(LDV_HANDLE ldv_handle, unsigned char neuron_id[], unsigned char program_id[]); 
bool ftp_xfer(LDV_HANDLE ldv_handle, unsigned char ftp_data[][36], int total_packets);
bool ftp_xfer_sign(LDV_HANDLE ldv_handle, unsigned char ftp_data[][36], int total_packets);
bool query_node_status(LDV_HANDLE ldv_handle);
bool query_node_domain_information(LDV_HANDLE ldv_handle);
bool start_sign_selftest(LDV_HANDLE ldv_handle);
bool stop_sign_selftest(LDV_HANDLE ldv_handle);
bool query_node_software_versions(LDV_HANDLE ldv_handle);

//  Global Variables
int g_address_mode=NI_SUBNET_NODE, g_node=3, g_subnet=1, g_domain=0;
unsigned char neuron_id[6] = {0,0,0,0,0,0};
unsigned char program_id[8] = {0,0,0,0,0,0,0,0};

//  Main Event
int main(int argc, char *argv[])   {
    LDV_STATUS ldv_status;
	LDV_HANDLE ldv_handle;
    unsigned char msg_code;   
	unsigned char msg_data[36];
	ExpAppBuffer ldv_msg;       // Sizeof = 257 
    int nv_size[14] = {12, 2, 3, 2, 7, 7, 7, 7, 7, 27, 29, 31, 6, 1 };
 	int i, total_packets;
    char c, r, j;
	int what_symbol, what_test;
    char serial_number[10];
    unsigned char ftp_data[][36] =
//        Line#0  STX  ADDR   CS    CR     LF    
         {{0x00, 0x02, 0x00, 0x02, 0x0D, 0x0A, 0xFE},        // 0x02 signiifies start of xmission
//        Line#1  RS     \    0      7     x   L/E  
          {0x01, 0X1E, 0x5C, 0x4F, 0x37, 0x72, 0x31, 0xFE},  //  RS \O7r1
//        Line#2  \      X     2    5     3     START 
          {0x02, 0x5C, 0x58, 0x32, 0x35, 0x33, 0xBE,
//                 F    L      U     S     H     I     N     G
//               0x46, 0x4C, 0x55, 0x53, 0x48, 0x49, 0x4E, 0X47,
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,      
//                 -     E    X     P      R     E     S     S
//               0x2D, 0x45, 0x58, 0x50, 0x52, 0x45, 0x53, 0x53, 
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,   
                 0x20, 0x20, 0x20, 0x20, 
//               END      
                 0xBF, 0xE9, 0x0D, 0x0A, 0xFE},
//        "FLUSHING-MAIN ST"                                  
//        Line#4  RS    
          {0x03, 0x1E, 0x5C, 0x58, 0x32, 0x35, 0x34, 0xBE,
//                  F    L     U      S     H     I     N     G 
//               0x46, 0x4C, 0x55, 0x53, 0x48, 0x49, 0x4E, 0X47,
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
//                 -    M     A      I     N          S     T 
//                 0x2D, 0x4D, 0x41, 0x49, 0x4E, 0x20, 0x53, 0x54, 
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
                 0x20, 0x20, 0x20, 0x20,
//               END     CS     CR     LF  
                 0xBF, 0x57, 0x0D, 0x0A, 0xFE},
//         "FLUSHING-LOCAL"
//        Line#5  RS 
          {0x04, 0x1E, 0x5C, 0x58, 0x32, 0x35, 0x35, 0xBE, 
//                 F      L    U      S     H     I     N    G
//                 0x46, 0x4C, 0x55, 0x53, 0x48, 0x49, 0x4E, 0X47,
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
//                -    L      O     C     A    L  
//                 0x2D, 0x4C, 0x4F, 0x43, 0x41, 0x4C,
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
                 0x20, 0x20, 0x20, 0x20,
//               END    CS     CR    LF  
                 0xBF, 0x57, 0x0D, 0x0A, 0xFE},
//         Last    US    L#   CS     CR    LF   ETX
          {0x05, 0x1F, 0x05, 0x21, 0x0D, 0x0A, 0x03, 0xFF}};    // 0x03 signifies end of xmission
          //  0xFE signifies end of packet marker (Not in train packets)
		  //  0xFF signifies end of packet marker (Not in train packets)
          //  		
          //  Used only in our code for simplicity of transmission
          //
          //  STX ADDRESS CHECKSUM CR LF  
          //  RS '\O' '7' 'r' '1' 
          //  '\X' '2' '5' '4' 0xBE 'F' 'L' 'U' 'S' 'H' 'I' 
          //       'N' 'G' '-' 'M' 'A' 'I' 'N' ' ' 'S' 'T' '.' 0xBF CHECKSUM CR LF
          //  RS '\X' '2' '5' '4' 0xBE 'F' 'L' 'U' 'S' 'H' 'I'      
          //       'N' 'G' '-' 'L' 'O' 'C' 'A' 'L' 0xBF CHECKSUM CR LF
          //   US LINE_NUMBER CHECKSUM CR LF ETX            

         //  FLUSHING-MAIN ST
         //  FLUSHING EXP  

	unsigned char ftp_data_sign[][36] =
         {{0x00, 0x02, 0x00, 0x02, 0x0D, 0x0A, 0x1E,
                 0x5C, 0x4F, 0x37, 0x72, 0x32,
                 0x5C, 0x58, 0x41, 0x55, 0x34, 0xBE, 0x46, 0x4C, 0x55, 0x53, 0x48, 0x49,
                 0x4E, 0x47, 0x2D,
 0x4D, 0x41, 0x49, 0x4E, 0x20, 0x53, 0xFE},
          {0x01, 0x54, 0x2E, 0x20, 0xBF, 0xE9, 0x0D, 0x0A, 0x1E, 
                 0x5C, 0x58, 0x32, 0x35, 0x34, 0xBE, 0x46, 0x4C, 0x55, 0x53, 0x48, 0x49,
                 0x4E, 0x47, 0x2D, 0x45, 0x58, 0x50, 0x52, 0x45, 0x53, 0x53, 0x20, 0xBF, 0xFE},
          {0x02, 0x57, 0x0D, 0x0A, 0x1F, 0x02, 0x21, 0x0D, 0x0A, 0x03, 0xFF}};    


    // Fetching serial Number from command line argument
    strcpy(serial_number, argv[1]);

    //  Open the Lonworks Network Device
    printf("---------------------------------------------------------------------\n");
	printf ("Opening Lonwork Networkk device with s/n (%s)\n", serial_number);
	if ((ldv_status = ldv_open (serial_number, &ldv_handle)) != LDV_OK) {

		printf ("ERROR: Unable to open Lonworks USB Network Interface s/n (%s), status code %d\n", serial_number, ldv_status);
		return (FALSE);
	}

    clear_network_buffer(ldv_handle);


    printf("\n NOTE: This program is set up for the following:");
    printf("\n       Subnet = 1, Node = 3");
    printf("\n       Node 3 is the Left Side Pulled HIGH with No Jumper");
    printf("\n       NODE 4 is the Right Side Pulled LOW by the Jumper\n"); 

menu:
    printf("\n      Sign Utility Main Menu (v1.0)\n\n");
    printf("Z -> Set Address Mode\n");
    printf("G -> Get Node Service Pin\n");
    printf("R -> Reset Node\n");
    printf("N -> Query Node Status\n");
    printf("V -> Query Node Software Version(s)\n");
    printf("D -> Query Node Domain Information\n");
	printf("F -> File Transfer\n");
	printf("J -> File Transfer for Sign\n");
    printf("S -> Sign Self Test\n");
    printf("Y -> Download Node Firmware\n");
    printf("P -> Fetch Node Network Variable\n");
	printf("U -> Update Node Network Variable\n");
    printf("C -> Get Node Configuration Data\n");
    printf("K -> Clear Status\n");
    printf("M -> Set Node Mode\n");
	printf("X -> Exit\n");
    printf("CHOICE > ");
    c=getchar();   //  get choice
    do { } while (getchar() != '\n');    //  Clear out excess characters and <CR>

    switch(c)
    {
        // --------------------- Set Address Mode ------------------------------
        case 'Z':         
        case 'z':
       	{
    again:  printf("ENTER 1 for Subnet_Node Mode\n");
            printf("      2 for Neuron_ID Mode\n"); 
            printf("      3 for Broadcast Mode\n"); 
            printf("      127 for Local Mode\n"); 
            printf("      126 for Implicit Mode\n"); 
          //       'N' 'G' '-' 'M' 'A' 'I' 'N' 'n");
            printf("CHOICE > ");

            c=getchar();
            do { } while (getchar() != '\n');    //  Clear out excess characters and <CR>
            g_address_mode=(int)c-0x30;       // g_address_mode will be a global variable
            if(g_address_mode <1 || g_address_mode >3) goto again;

            printf("Address Mode set to %d\n", g_address_mode);           
            if(g_address_mode == NI_SUBNET_NODE)
            {
				//  Enter subnet & node
	 again1:    printf("Enter Node (Only 1 to 9 is Acceptable) -> ");
    	        c=getchar();
                do { } while (getchar() != '\n');    //  Clear out excess characters and <CR>	
                g_node=(int)c-0x30;
                if(g_node<1 || g_node >9) goto again1;
				g_subnet = 1;
				g_domain = 0;
                printf("Settings:  Domain-> 0   Subnet-> 1   Node-> %d\n",g_node); 
            }         
 	        else if(g_address_mode == NI_NEURON_ID)		
			{
                // manually get service pin
				manual_service_pin(ldv_handle, neuron_id, program_id);

    			printf("neuron_id  -> ");
    			for(i=0;i<=5;i++) printf("%02x ",neuron_id[i]);
    			printf("\nprogram_id -> ");
    			for(i=0;i<=7;i++) printf("%c",program_id[i]);
   				printf("\n");
			}				
       		break;
		}
		// -------------------- Manually Get Service Pin -----------------------
	    case 'G':
		case 'g': 
		{
			clear_network_buffer(ldv_handle);
			manual_service_pin(ldv_handle, neuron_id, program_id);

    		printf("neuron_id  -> ");
    		for(i=0;i<=5;i++) printf("%02x ",neuron_id[i]);
    		printf("\nprogram_id -> ");
    		for(i=0;i<=7;i++) printf("%c",program_id[i]);
   			printf("\n");
			break;
		}

        // --------------------------- Reset Node ------------------------------
        case 'r':
		case 'R':	
		{
		 	printf ("Ready to Reset Node.\n");                          
	     	if(!reset_node(ldv_handle))
	    		{
					printf ("main function: 'reset_node' Unsuccessful.\n");
				}
			printf("Reset in Progress. Press <Enter> when Reset has Completed .....");
			getchar();
			break; 
		}      
		// ------------------- File Transfer Section ---------------------------    
		case 'F':
		case 'f':
		{
	again2: printf("ENTER 1 for Circle (LOCAL)\n");            
			printf("      2 for Diamond (EXPRESS)\n");
            printf("      3 to Return to Main Menu\n");
            printf("CHOICE -> ");

            c=getchar();
            do { } while (getchar() != '\n');    //  Clear out excess characters and <CR> 

            what_symbol=(int)c-0x30;       // g_address_mode will be a global variable
            if(what_symbol <1 || what_symbol >3) goto again2; 
            if(what_symbol==3) break;
            ftp_data[1][6] = c;

            printf("ENTER ROUTE -> ");
            r=getchar();
            do { } while (getchar() != '\n');    //  Clear out excess characters and <CR> 			
            ftp_data[1][4] = toupper(r);

            // blanks in ftp_data
        	for(i=7;i<=26;i++) 
           	{
               	ftp_data[2][i] = 0x20;
            }    

            // get text1
            printf("                              [....................]\n");  
            printf("Enter TEXT 1 [20 CHARS MAX] -> ");
            for(i=7;i<=26;i++) 
           	{
               	c=getchar();   
                if(c != '\n') {ftp_data[2][i] = toupper(c);}
                else  i=29;
            }
 	        if (i!=30) do { } while (getchar() != '\n');    //  Clear out excess characters and <CR> 

            // Get text2
        	for(i=8;i<=27;i++) 
           	{
               	ftp_data[3][i] = 0x20;
            }     
            printf("                              [....................]\n");
            printf("Enter TEXT 2 [20 CHARS MAX] -> ");
            for(i=8;i<=27;i++) 
           	{
               	c=getchar();           		
                if(c != '\n') {ftp_data[3][i] = toupper(c);}
                else  i=29;
            }
 	       if (i!=30) do { } while (getchar() != '\n');    //  Clear out excess characters and <CR> 

            // Get text3
        	for(i=8;i<=27;i++) 
           	{
               	ftp_data[4][i] = 0x20;
            }    

            printf("                              [....................]\n");
            printf("Enter TEXT 3 [20 CHARS MAX] -> ");
            for(i=8;i<=27;i++) 
           	{
               	c=getchar();           		
                if(c != '\n') ftp_data[4][i] = toupper(c);
                else  i=29;
            }
	        if (i!=30) do { } while (getchar() != '\n');    
            //  Clear out excess characters and <CR> 

		 	printf ("Ready to Transfer File.\n");
			clear_network_buffer(ldv_handle);		    
			total_packets = 6;
			ftp_xfer(ldv_handle, ftp_data, total_packets);	
			goto again2;
		}
		// ------------------- File Transfer Section ---------------------------    
		case 'J':
		case 'j':
			//break;	
		{
		 	printf ("Ready to Transfer File.\n");
			clear_network_buffer(ldv_handle);		    
			total_packets = 3;
			ftp_xfer_sign(ldv_handle, ftp_data_sign, total_packets);	
			break;
		}
        // ----------------------- Query Node Status ---------------------------
        case 'N':
        case 'n':
        {
			clear_network_buffer(ldv_handle);	
		 	printf ("Ready to Query Node Status.\n");                          
	     	if(!query_node_status(ldv_handle))
	    		{
					printf ("main function: 'query_node_status' Unsuccessful.\n");
				}
			break; 
        }
		// ------------------- Query Node Domain Information -------------------
		case 'd':
		case 'D':
		{
			clear_network_buffer(ldv_handle);	
		 	printf ("Ready to Query Node Domain Information.\n");                          
	     	if(!query_node_domain_information(ldv_handle))
	    		{
					printf ("main function: 'query_node_domain_information' Unsuccessful.\n");
				}
			break;
		}
		case 's':
		case 'S':
		{
	again3:	printf("ENTER 1 to Start Self-Test\n");            
			printf("      2 to Stop Self-Test\n"); 
    		printf("      3 to Return to Main Menu\n");
            printf("CHOICE > ");

            c=getchar();
            do { } while (getchar() != '\n');    //  Clear out excess characters and <CR> 
            what_test=(int)c-0x30;       // g_address_mode will be a global variable
            if(what_test <1 || what_test >3) goto again3; 
			if(what_test==3) break;            

			switch(what_test)
				{
        	   		case 1: 
   						{
       		   		   		start_sign_selftest(ldv_handle);
                       		break;
						}
                	case 2:
						{		
   			  				stop_sign_selftest(ldv_handle);
							break;
						}
				}
            goto again3; 
		}

		case 't':
		case 'T':
		{
			stop_sign_selftest(ldv_handle);
            break;
		}
		case 'x':
		case 'X':
		{
			return (lon_close(ldv_handle));
            return;
		}
        // ------------------- Query Node Software Versions -------------------
		case 'v':
		case 'V':
		{
			query_node_software_versions(ldv_handle);
            break;			
		}
	} 

	goto menu;


 }

//------------------------------ End of main function --------------------------


//******************************************************************************
//
//    Function:     bool clear_network_buffer(LDV_HANDLE ldv_handle)
//
//    Description:  This function clears the Lonworks network interface of
//                  any impending messages.
//
//******************************************************************************
bool clear_network_buffer(LDV_HANDLE ldv_handle)
	 {
		LDV_STATUS ldv_status = LDV_NO_MSG_AVAIL; 	
		ExpAppBuffer ldv_msg;
	    int count = 0;
	    bool done = FALSE;     
	
		memset (&ldv_msg, 0, sizeof(ExpAppBuffer));    // Clear ldv message buffer

    	//  Keep looping until all messages have been read
		while(!done)
			{
			    if ((ldv_status = ldv_read (ldv_handle, &ldv_msg, sizeof(ExpAppBuffer))) == LDV_NO_MSG_AVAIL)
					{
						done = TRUE; 
					}
				else count++;
			}

		printf("There were %d messages in the network buffer.\n", count);
 		return done;
	}

//******************************************************************************
//
//    Function:     bool manual_service_pin(LDV_HANDLE ldv_handle, 
//                                          char neuron_id[], char program_id[])
//
//    Description:  This function retieves the service pin message from the 
//                  Lonworks interface. It fills the neuron_id and program_id
//                  buffers with the corerct data. 
//
//******************************************************************************
bool manual_service_pin(LDV_HANDLE ldv_handle, unsigned char neuron_id[], unsigned char program_id[]) 
	{
		LDV_STATUS ldv_status = LDV_NO_MSG_AVAIL; 	
		ExpAppBuffer ldv_msg;
	    bool received = FALSE;     
		int i, count = 0;

        //  Clear out all buffers
        for(i=0; i<6; i++) neuron_id[i] = 0x00;
        for(i=0; i<8; i++) program_id[i] = 0x00;
 		memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     // Clear ldv buffer
        
        printf("Press the Service Pin on UUT:\n");

    	while(!received & count < 500)

			{
			    if ((ldv_status = ldv_read (ldv_handle, &ldv_msg, sizeof(ExpAppBuffer))) == LDV_OK)
					{
						//  A message was received.  Check to see if it is
 						//  a service pin message and then decode it.                        
						//  The message code for a service pin message is 0x'7F'
                        if(ldv_msg.data.exp.code == 0x7F)
							{			
		   	                    printf("Service Pin Message Received.\n");				
								received = TRUE;  
		                        //display_packet((char *)&ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr));
                                //  Put received data in arrays
		                        for(i=0;i<=5;i++) neuron_id[i] = ldv_msg.data.exp.data[i];
								for(i=0;i<=7;i++) program_id[i] = ldv_msg.data.exp.data[i+6];     
							}
                        //  Something was received, but not what we want
                        else count++; 	                                       
					}
				else
					{
        	          	if(ldv_status == LDV_NO_MSG_AVAIL) count++;
					}
			}

		if(!received) printf("No Service Pin Message Received.\n");
		return (received);		
	}

//******************************************************************************
//
//    Function:     bool query_node_status(LDV_HANDLE ldv_handle)
//
//    Description:  This function retrieves the LON Node status
//                 
//
//******************************************************************************
bool query_node_status(LDV_HANDLE ldv_handle)
	{
		LDV_STATUS ldv_status;
	    ExpAppBuffer ldv_msg;
		unsigned char msg_code = 0x51;  // Query node status  
		unsigned char msg_data[1] = {0x00};
		unsigned char addr_mode, addr_data[11];

		printf("Entering function 'query_node_status'.\n");

	 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
		if (!lon_write_msg (ldv_handle, niNTQ, REQUEST, msg_code, msg_data, 1, addr_mode, addr_data))
	        {
				printf ("function 'query_node_status': ERROR (LON Write Unsuccessful).\n");
				return FALSE;      
			}

	    //  Read Back Data as a result of the above request
	 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));  // clear 257 byte buffer
		if (!lon_read_msgs (ldv_handle, &ldv_msg)) 
		{
			printf ("function query_node_status: ERROR (LON Read Unsuccessful).\n");
			return FALSE;
		}
		else 
		{
            printf("\nDevice Status ..............\n");
         	if(ldv_msg.data.exp.code == 0x31) 
				{
					printf("Last Reset Cause: %02x\n", ldv_msg.data.exp.data[10]);
					printf("Device State: %02x\n", ldv_msg.data.exp.data[11]);
					printf("Firmware Version Number: %02x\n", ldv_msg.data.exp.data[12]);	
					printf("Last Error Logged: %02x\n", ldv_msg.data.exp.data[13]);
					printf("Neuron Model number: %02x\n", ldv_msg.data.exp.data[14]);
				}
			else
				{
					printf("Invalid Success Response.\n");
				}
			printf("Ok ..............\n\n");
	       }

        //  Read Back Completion Event
	 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
		if (!lon_read_msgs (ldv_handle, &ldv_msg)) 
			{
				printf ("function 'query_node_status': ERROR (LON Read Unsuccessful).\n");
			}

		printf("Leaving function 'query_node_status'.\n");
		return TRUE;

	}

//******************************************************************************
//
//    Function:     bool query_node_domain_information(LDV_HANDLE ldv_handle)
//
//    Description:  This function retrieves the LON Domain Information
//                 
//
//******************************************************************************
bool query_node_domain_information(LDV_HANDLE ldv_handle)
	{
		LDV_STATUS ldv_status;
	    ExpAppBuffer ldv_msg;
		unsigned char msg_code = 0x6A;  // Query node domain information  
		unsigned char msg_data[2] = {0x00, 0x01};
		unsigned char addr_mode, addr_data[11];
		int i, index;

		printf("Entering function 'query_node_domain_information'.\n");

        for(index=0; index<=1;index++)
		{
		printf("Querying for index = %d\n",index);
	 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
		if (!lon_write_msg (ldv_handle, niTQ, REQUEST, msg_code, msg_data+index, 1, addr_mode, addr_data))
	        {
				printf ("function 'query_node_domain_information': ERROR (LON Write Unsuccessful).\n");
				return FALSE;      
			}

	    //  Read Back Data as a result of the above request
	 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));  // clear 257 byte buffer
		if (!lon_read_msgs (ldv_handle, &ldv_msg)) 
		{
			printf ("function query_node_domain_information: ERROR (LON Read Unsuccessful).\n");
			return FALSE;
		}
		else 
		{
            printf("\nDevice Domain Information ..............\n");
         	if(ldv_msg.data.exp.code == 0x2A) 
				{
					printf("Index: %02d\n", index); 
                    if(ldv_msg.data.exp.data[8] == 0x80)
					{
						printf("Size: Unused\n");
					}
					else 
					{
						printf("Size: %02x\n", ldv_msg.data.exp.data[8]);
						printf("Subnet: %02x\n", ldv_msg.data.exp.data[6]);
						printf("Node: %02x\n", ldv_msg.data.exp.data[7]&0x7F);	
						printf("Authentication Key: ");
    	                for(i=9;i<=14;i++) printf("%02x ", ldv_msg.data.exp.data[i]);
						printf("\n");
						printf("Domain ID: ");
    	                for(i=0;i<ldv_msg.data.exp.data[8];i++) printf("%02x ", ldv_msg.data.exp.data[i]);
						printf("\n");
					}
				}
			else
				{
					printf("Invalid Success Response.\n");
				}
			printf("OK ..............\n\n");
	       }

        //  Read Back Completion Event
	 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
		if (!lon_read_msgs (ldv_handle, &ldv_msg)) 
			{
				printf ("function 'query_node_domain': ERROR (LON Read Unsuccessful).\n");
			}
		}

		printf("Leaving function 'query_node_domain_information'.\n");
		return TRUE;
       
	}

//******************************************************************************
//
//    Function:     bool reset_node(LDV_HANDLE ldv_handle)
//
//    Description:  This function resets the LON node by sending a reset command
//                  over the network.
//
//******************************************************************************
bool reset_node(LDV_HANDLE ldv_handle)
	{
		LDV_STATUS ldv_status;
	    ExpAppBuffer ldv_msg;
		unsigned char msg_code = 0x6c;   //  Set Node Mode
		unsigned char msg_data[2] = {0x02, 0x00};
		unsigned char addr_mode, addr_data[11];

		printf("Entering function 'reset_node'.\n");

	 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
		if (!lon_write_msg (ldv_handle, niNTQ, UNACKD, msg_code, msg_data, 2, addr_mode, addr_data))
	        {
				printf ("function 'reset_node': ERROR (LON Write Unsuccessful).\n");
				return FALSE;      
			}

	    //  Read Back Completion Event
	 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
		if (!lon_read_msgs (ldv_handle, &ldv_msg)) 
			{
				printf ("function 'reset_node': ERROR (LON Read UnSuccessful).\n");
                return FALSE;
			}

		printf("Leaving function 'reset_node'.\n");
		return TRUE;
	} 

//******************************************************************************
//
//    Function:     bool start_sign_selftest(LDV_HANDLE ldv_handle)
//
//    Description:  This function starts the sign self-test
//

//******************************************************************************
bool start_sign_selftest(LDV_HANDLE ldv_handle)
	{
		LDV_STATUS ldv_status = LDV_NO_MSG_AVAIL; 	
	    boolean done;     
		int i;
    	unsigned char msg_data[36];
		ExpAppBuffer ldv_msg;           //  Sizeof(ExpAppBuffer) = 257 	
		unsigned char addr_mode, addr_data[11];


        //----------------------------------------------------------------------
        //                  
		//----------------------------------------------------------------------

	    for(i=0; i<=35; i++) msg_data[i] = 0x00;
	    msg_data[0] = 0x65;
	    msg_data[1] = 0xFF;
	

		printf ("Ready to start sign self-test.\n");
	    if(!update_nv(ldv_handle, 1, msg_data, 2))
			{
				printf ("SNVT_file_req NV Update Unsucessful. Exiting request\n");
				return FALSE;
			}

        return TRUE;	
	} 

//******************************************************************************
//
//    Function:     bool stop_sign_selftest(LDV_HANDLE ldv_handle)
//
//    Description:  This function stops the sign self-test
//

//******************************************************************************
bool stop_sign_selftest(LDV_HANDLE ldv_handle)
	{
		LDV_STATUS ldv_status = LDV_NO_MSG_AVAIL; 	
	    boolean done;     
		int i;
    	unsigned char msg_data[36];
		ExpAppBuffer ldv_msg;           //  Sizeof(ExpAppBuffer) = 257 	
		unsigned char addr_mode, addr_data[11];

        //----------------------------------------------------------------------
        //                     
		//----------------------------------------------------------------------

	    for(i=0; i<=35; i++) msg_data[i] = 0x00;
	    msg_data[0] = 0x66;
	    msg_data[1] = 0xFF;
	

		printf ("Ready to stop sign self-test.\n");
	    if(!update_nv(ldv_handle, 1, msg_data, 2))
			{
				printf ("SNVT_file_req NV Update Unsucessful. Exiting request\n");
				return FALSE;
			}

        return TRUE;	
	} 

//******************************************************************************
//
//    Function:     bool query_node_sofware_versions(LDV_HANDLE ldv_handle)
//
//    Description:  This function retreives the node's software versions
//

//******************************************************************************
bool query_node_software_versions(LDV_HANDLE ldv_handle)
	{
		LDV_STATUS ldv_status = LDV_NO_MSG_AVAIL; 	
	    boolean done;     
		int i;

    	unsigned char msg_data[36];
		ExpAppBuffer ldv_msg;           //  Sizeof(ExpAppBuffer) = 257 	
		unsigned char addr_mode, addr_data[11];

        //----------------------------------------------------------------------
        //                     
		//----------------------------------------------------------------------

	    for(i=0; i<=35; i++) msg_data[i] = 0x00;
	    msg_data[0] = 0x69;
	    msg_data[1] = 0xFF;
	

		printf ("Ready to retrieve the node's software versions.\n");
	    if(!update_nv(ldv_handle, 1, msg_data, 2))
			{
				printf ("function 'query_node_software_versions': update_nv Unsucessful. Exiting request\n");
				return FALSE;
			}


        //  Results are returned in nv11
        // Retreive nv11 and decode  
        //  returned value is: 00 01 01 (Is this correct?)
		printf ("Ready to retrieve the node's software versions.\n");

	    for(i=0; i<=35; i++) msg_data[i] = 0x00;
	    if(!retrieve_nv(ldv_handle, 11, msg_data, 31))
			{
				printf ("function 'query_node_software_versions': update_nv Unsucessful. Exiting Request\n");
				return FALSE;
			}        
        else
            {
                printf("\n Symbol Software Version -> %1d.%1d\n", msg_data[3], msg_data[4]);
            }
        
        return TRUE;	
	} 

//******************************************************************************
//
//    Function: int update_nv(LDV_HANDLE ldv_handle, unsigned char index, 
//                            unsigned char* data, int size)
//
//    Description:
//
//
//******************************************************************************
bool update_nv(LDV_HANDLE ldv_handle, int index, unsigned char data[], int size)
  {
	LDV_STATUS ldv_status;
    ExpAppBuffer ldv_msg;
    unsigned char msg_code = 0x68;   			
    unsigned char msg_data[35];   
    unsigned char nv_selector_high, nv_selector_low;
	int i, count;
	unsigned char addr_mode;
	unsigned char addr_data[11];
  
    printf("Entering Function: update_nv.\n");  
    
	//--------------------------------------------------------------------------
    //        PART 1 - Query Network Variable Configuration Table
	//--------------------------------------------------------------------------
 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer)); // clear 257 byte buffer    
    msg_data[0] = (unsigned char)index;
 
    //  We need to find out the Selector from the Table
    //  in order to update the variable   
	if (!lon_write_msg (ldv_handle, niTQ, REQUEST, 0x68, msg_data, 1, addr_mode, addr_data))
		{
    		printf ("function 'update_nv': ERROR (LON Write Unsuccessful).\n");
			return FALSE;      
		}

    //  Read Back Data as a result of the above request
 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));  // clear 257 byte buffer
	if (!lon_read_msgs (ldv_handle, &ldv_msg)) 		
		{
    		printf ("function 'update_nv': ERROR (LON Read Unsuccessful).\n");
			return FALSE;
        }
	else 
		{
            //  Decode high and low selectors here
            if(ldv_msg.data.exp.code == 0x28) 
              {
				printf("function 'update_nv': Ready to Decode NV Selector.\n");
                nv_selector_high =  ldv_msg.data.exp.data[0] & 0x3f;
        	          	if(ldv_status == LDV_NO_MSG_AVAIL) count++;
				nv_selector_low = ldv_msg.data.exp.data[1];
              }
            else
              {
				printf("function 'update_nv': Received Message, but code not what expected.\n");
                return FALSE;
              }   
        }
      
    //  Read Back Completion Event
 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));  // clear out 257 byte buffer
	if (!(ldv_status=lon_read_msgs (ldv_handle, &ldv_msg))) 
		{
    		printf ("function 'update_nv': ERROR (LON Read Unsuccessful).\n");
			return FALSE;
        }
	else 
		{
             //  Check here to make sure message code is 0x68
             printf("function 'update_nv': LON Read Request Succcesful.\n");
        }

	//--------------------------------------------------------------------------
    //  Part 2 - Ready to update the network variable
	//--------------------------------------------------------------------------

    //  If we do a poll here, we can find out the data size of the variable.  
	//  Is that necessary? Right now it's hard coded in the main function and passed
    // as a function parameter. 

    msg_code = 0x80 | nv_selector_high; 		
    msg_data[0] = nv_selector_low;
    for(i=1; i<=size;i++) msg_data[i] = data[i-1];

	if (!lon_write_msg (ldv_handle, niTQ, ACKD, msg_code, msg_data, size+1, addr_mode, addr_data))
		{
			printf ("function 'update_nv': ERROR (LON Write Unsucessful).\n");
			return FALSE;      
		}
	else
		{
            printf("function 'update_nv': LON Write Succcesful.\n");
        }   


	printf("Exiting Function: update_nv\n");
    return (TRUE);

	//  For some reason on this set of signs, the network variable response is 
	//  being read back before the message completion event.
	//  so we will temporaily skip this 

    //  Read Back Completion Event
 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));   // clear out 257 byte buffer
	if (!(ldv_status=lon_read_msgs (ldv_handle, &ldv_msg))) 
		{
			printf ("function 'update_nv': ERROR (LON Read Unsucessful).\n");
			return FALSE;
		}
	else 
		{
             printf("function 'update_nv': LON Read Succcesful.\n");
        }

    printf("Exiting Function: update_nv\n");
        	          
    return (TRUE);
  } 

//******************************************************************************
//
//    Function: int retrieve_nv(LDV_HANDLE ldv_handle, unsigned char index, 
//	  unsigned char* data, int size)
//
//    Description:
//
//
//******************************************************************************
bool retrieve_nv(LDV_HANDLE ldv_handle, int index, unsigned char data[], int size)
  {
	LDV_STATUS ldv_status;
    ExpAppBuffer ldv_msg;
    unsigned char msg_code = 0x73;   
    unsigned char msg_data[36];   
	int i; 
    unsigned char addr_mode, addr_data[11];
  
    printf("Entering function: retrieve_nv.\n");  
    
	//--------------------------------------------------------------------------
    //                       Fetch Network Variable 
	//--------------------------------------------------------------------------

 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer)); // clear 257 byte buffer    
    msg_data[0] = (unsigned char)index;
 

	if (!lon_write_msg (ldv_handle, niTQ, REQUEST, msg_code, msg_data, 1, addr_mode, addr_data))
		{
			printf ("function 'retrieve_nv: ERROR (LON Write Unsucessful).\n");
			return FALSE;      
		}
	else

		{
            //printf("LON write Succcesful.\n");
        }   

    //  Read Back Data as a result of the above request
 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));  // clear 257 byte buffer
	if (!lon_read_msgs (ldv_handle, &ldv_msg)) 
		{
			printf ("function 'retrieve_nv': ERROR (LON Read Unsucessful).\n");
			return FALSE;
          }
	else 
		{
            //  Decode high and low selectors here
            if(ldv_msg.data.exp.code == 0x33) 

              {
                for(i=0;i<32;i++) data[i] = ldv_msg.data.ldv.data[i];
              }
            else
              {
				printf("function 'retrieve_nv': Response not what expected.\n");
              }   
        }
      
    //  Read Back Completion Event
 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));  // clear out 257 byte buffer
	if (!(ldv_status=lon_read_msgs (ldv_handle, &ldv_msg))) 
		{
			printf ("function 'retrieve_nv': ERROR (LON Read Unsucessful).\n");
			return FALSE;
          }
	else 
		{
             //printf("LON read Succcesful.\n");
             //  check if  
        }

    printf("Exiting function 'retrieve_nv'.\n");

    return (TRUE);
  } 

//******************************************************************************
//
//    Function: bool ftp_xfer(LDV_HANDLE ldv_handle, unsigned char* data, 
//                           int total_packets
//
//    Description:
//

//
//******************************************************************************
bool ftp_xfer(LDV_HANDLE ldv_handle, unsigned char ftp_data[][36], int total_packets)
	{
		LDV_STATUS ldv_status = LDV_NO_MSG_AVAIL; 	
	    boolean done;     
		int i, j, k, window, packet, count;
    	unsigned char msg_data[36];
        unsigned char msg_code = 0x3E;  //  ftp tranfer code
		unsigned char service;          //  type of service
		ExpAppBuffer ldv_msg;           //  Sizeof(ExpAppBuffer) = 257 	
        int total_windows, packet_count;	
		unsigned char addr_mode, addr_data[11];
		boolean no_more_packets = FALSE;

        //----------------------------------------------------------------------
        //                     PART I - FTP Initiation
		//----------------------------------------------------------------------
        //  packet 1: 0x12 0x10 ... (Downlink Message)
        //  packet 2: 0x16 0x16 ... (Uplink Response Messages & Completion Codes)
		//  packet 3: 0x16 0x16 ... (Uplink Response Messages & Completion Codes)
        //  packet 4: 0x12 0xxx ... (Downlink Message)
		//  packet 5: 0x16 0xxx ... (Uplink Response Messages & Completion Codes)
        //  packet 6: 0x18 0xxx ... (Uplink Message Received from the network)  	   

		//  Initiate ftp transfer by sending SNVT_file_req (12 Bytes) to OPEN 
	    for(i=0; i<=35; i++) msg_data[i] = 0x00;
	    msg_data[0] = 0x01;
	    msg_data[1] = 0x00;
		msg_data[2] = 0x03;
	    msg_data[3] = 0x01;
	    msg_data[4] = 0xF4;
	    msg_data[5] = 0x00;


        done = FALSE;
        count = 0;
		printf ("ftp_xfer: Ready to send 'SNVT_file_req' to OPEN file transfer.\n");
	    if(!update_nv(ldv_handle, 0, msg_data, 12))
			{
				printf ("ftp_xfer: SNVT_file_req NV Update Unsucessful. Exiting request\n");
				return FALSE;
			}
	
        //printf("We are now waiting for 'SNVT_file_status' response from node.\nPress RETURN to Continue.");
        //getchar(); 	

        //  Wait for 'SVNT_file_status' response from Node 
 		memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
    	while(!done & (count < 1000))

			{
			    if ((ldv_status = ldv_read (ldv_handle, &ldv_msg, sizeof(ExpAppBuffer))) == LDV_OK)
					{
						//  A message was received, Check to see if NV response which must be 0b1xxxxxxx.                        
						if(ldv_msg.data.unv.must_be_one == 0x01)
							{			
    		   	                printf("ftp_xfer: Network Variable Packet Received from Network.\n");			
								done = TRUE;  
		                        display_packet((char *)&ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr));
							}
                        //  Something else was received, keep trying
                        else count++; 	                                       
					}
				else
					{
        	          	if(ldv_status == LDV_NO_MSG_AVAIL) count++;
					}
			}
        	
       //  If we didn't received the 'SNVT_file_status' response, we can't continue  
       if(!done) 
            {
	          printf("ftp_xfer: Network Variable SNVT_file_status Response NOT received (!done).\n");
              return FALSE;
			}

        //----------------------------------------------------------------------
        //                PART II - TRANSFER OF FTP DATA PACKETS
        //----------------------------------------------------------------------
        printf("ftp_xfer: We are now Ready to transfer data to sign.\nPress Return to Start Transfer:\n\n");
        //getchar();

        clear_network_buffer(ldv_handle);
        total_windows = (total_packets-1)/6 +1;
        packet_count = 0;

        for(window=0; window<total_windows; window++)
			{
				for(packet=0; packet<6; packet++)
 					{         
						for(k=1; (ftp_data[packet][k] != 0xFE && ftp_data[packet][k] != 0xFF); k++) 
							{ 
                            	msg_data[k] = ftp_data[packet][k];
							}
						if(ftp_data[packet][k] == 0xFF) no_more_packets = TRUE; 

                        msg_data[0] = (unsigned char)(window*16 + packet);
                        packet_count++;

                        if((packet_count == total_packets) || (packet == 5))
							{
                                service = REQUEST;
								packet = 6;   // ensures exit from packet loop

                            }
						else
							{
								service = UNACKD;
							}							

      					if (!lon_write_msg (ldv_handle, niTQ, service, 0x3E, msg_data, k, addr_mode, addr_data))
							{ 
								printf ("ftp_xfer: LON write Unsucessful. Application aborting.\n");
								return FALSE;      
                    		}
						else//clear_network_buffer(ldv_handle);
        
							{
                                if(packet!=6)					            
								    printf("ftp_xfer: LON packet [%d] write Succcesful.\n", packet+1);
								else 
									printf("ftp_xfer: LON packet [final] write Sucessful.\n");
                                    //getchar();
                                    //clear_network_buffer(ldv_handle);
					        }  
					}
			}

        //clear_network_buffer(ldv_handle);
        //  After last packet, we expect a response
        printf("ftp_xfer: Let's read ftp packet transfer responses - Expecting %d.\n",total_packets);
        for(i=0; i<=total_packets;i++) 
		{
			memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
        	done = FALSE;
        	count = 0;
    		while(!done & (count < 750))
			{
			    if ((ldv_status = ldv_read (ldv_handle, &ldv_msg, sizeof(ExpAppBuffer))) == LDV_OK)
					{
						//  A message was received, Check to see if NV response                        
						if(TRUE)
							{			
		   	                    printf("ftp_xfer: FTP response packet [%d] received.\n",i+1);				
								done = TRUE;  
		                        display_packet((char *)&ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr));
							}
                        //  Something else was received, Retry
                        else count++; 	                                       
					}
				
				else
					{
        	          	if(ldv_status == LDV_NO_MSG_AVAIL) count++;
					}
			}
         }
       
        //clear_network_buffer(ldv_handle);
        printf("ftp_xfer: Data transfer to sign complete.\n Press Enter to Continue:\n\n");
		//getchar();
		//----------------------------------------------------------------------
        //                         PART III - FTP Conclusion 
		//-----------------------------------------------------------------------

        printf("ftp_xfer: Ready to send 'SNVT_file_req' to CLOSE transfer.\n\n");
 
        //  Conclude ftp transfer by sending SNVT_file_req (12 Bytes) to CLOSE       
	    for(i=0; i<=35; i++) msg_data[i] = 0x00;
	    msg_data[0] = 0x02;
	    msg_data[1] = 0x00;
		msg_data[2] = 0x03;
	    msg_data[3] = 0x01;
	    msg_data[4] = 0xF4;
	    msg_data[5] = 0x00;


        done = FALSE;
        count = 0;
	    if(!update_nv(ldv_handle, 0, msg_data, 12))
			{
				printf ("ftp_xfer: SNVT_file_req NV Unsucessful.\n");
				return FALSE;

			}

        printf("We are now waiting for 'SNVT_file_status' response from node.\nPress RETURN to Continue.\n");
        //getchar();    
        // Wait for Network Variable response, SVNT_file_status, from Node 
 		memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
    	while(!done & (count < 1000))

			{
			    if ((ldv_status = ldv_read (ldv_handle, &ldv_msg, sizeof(ExpAppBuffer))) == LDV_OK)
					{
						//  A message was received, Check to see if NV response                        
						if(ldv_msg.data.unv.must_be_one == 0x01)
							{			
		   	                    printf("ftp_xfer: Network Variable SNVT_file_status Response received.\n");				
								done = TRUE;  
		                        display_packet((char *)&ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr));
							}
                        //  Something else was received, Retry
                        else count++; 	                                       
					}
				else
					{
        	          	if(ldv_status == LDV_NO_MSG_AVAIL) count++;
					}
			}


		//  We didn't received the SNVT_file_status Response, we can't continue  		
		if(!done) 
            {
	          printf("ftp_xfer: Network Variable SNVT_file_status Response NOT received (!done)\n");
              return FALSE;
			}       

		return TRUE;
	}

//******************************************************************************
//
//    Function: bool ftp_xfer_sign(LDV_HANDLE ldv_handle, unsigned char* data, 
//                           int total_packets
//
//    Description:
//
//
//******************************************************************************
bool ftp_xfer_sign(LDV_HANDLE ldv_handle, unsigned char ftp_data[][36], int total_packets)
	{
		LDV_STATUS ldv_status = LDV_NO_MSG_AVAIL; 	
	    boolean done;     
		int i, j, k, window, packet, count;
    	unsigned char msg_data[36];
        unsigned char msg_code = 0x3E;  //  ftp tranfer code
		unsigned char service;          //  type of service
		ExpAppBuffer ldv_msg;           //  Sizeof(ExpAppBuffer) = 257 	
        int total_windows, packet_count;	
		unsigned char addr_mode, addr_data[11];
		boolean no_more_packets = FALSE;

        //----------------------------------------------------------------------
        //                     PART I - FTP Initiation
		//----------------------------------------------------------------------
        //  packet 1: 0x12 0x10 ... (Downlink Message)
        //  packet 2: 0x16 0x16 ... (Uplink Response Messages & Completion Codes)
		//  packet 3: 0x16 0x16 ... (Uplink Response Messages & Completion Codes)
        //  packet 4: 0x12 0xxx ... (Downlink Message)
		//  packet 5: 0x16 0xxx ... (Uplink Response Messages & Completion Codes)
        //  packet 6: 0x18 0xxx ... (Uplink Message Received from the network)  	   

		//  Initiate ftp transfer by sending SNVT_file_req (12 Bytes) to OPEN 
	    for(i=0; i<=35; i++) msg_data[i] = 0x00;
	    msg_data[0] = 0x01;
	    msg_data[1] = 0x00;
		msg_data[2] = 0x03;
	    msg_data[3] = 0x01;
	    msg_data[4] = 0xF4;
	    msg_data[5] = 0x00;


		printf ("Ready to send 'SNVT_file_req' to OPEN file transfer.\n");
	    if(!update_nv(ldv_handle, 0, msg_data, 12))
			{
				printf ("SNVT_file_req NV Update Unsucessful. Exiting request\n");
				return FALSE;//ftp_xfer_sig
			}
	
        //printf("We are now waiting for 'SNVT_file_status' response from node.\nPress RETURN to Continue.");
        //getchar(); 	

        //  Wait for 'SVNT_file_status' response from Node 
 		memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
        done = FALSE;
        count = 0;
    	while(!done & count < 750)

			{
			    if ((ldv_status = ldv_read (ldv_handle, &ldv_msg, sizeof(ExpAppBuffer))) == LDV_OK)
					{
						//  A message was received, Check to see if NV response which must be 0b1xxxxxxx.                        
						if(ldv_msg.data.unv.must_be_one == 0x01)
							{			
    		   	                printf("Network Variable Packet Received from Network.\n");			
								done = TRUE;  
		                        display_packet((char *)&ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr));
							}
                        //  Something else was received, keep trying - Might be the 
                        else {

								display_packet((char *)&ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr)); count++;
							 }
					}
			
				else
					{
        	          	if(ldv_status == LDV_NO_MSG_AVAIL) count++;
		 				//printf("No Message available.\n");
						//display_packet((char *)&ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr));
					}
			}
        	
		done=TRUE;
       //  If we didn't received the 'SNVT_file_status' response, we can't continue  
       if(!done) 
           {
	          printf("Network Variable SNVT_file_status Response NOT received.\n");
              return FALSE;
		   }
        
 
        //----------------------------------------------------------------------
        //                PART II - TRANSFER OF FTP DATA PACKETS
        //----------------------------------------------------------------------
        printf("We are now Ready to transfer data to sign.\nPress Return to Start Transfer:\n\n");
        //getchar();

        clear_network_buffer(ldv_handle);
        total_windows = (total_packets-1)/6 +1;
        packet_count = 0;

        for(window=0; window<total_windows; window++)
			{
				for(packet=0; packet<6; packet++)
 					{         
						for(k=1; ftp_data[packet][k] != 0xFE && ftp_data[packet][k] != 0xFF; k++) 
							{ 
                            	msg_data[k] = ftp_data[packet][k];
							}
						if(ftp_data[packet][k] == 0xFF) no_more_packets = TRUE; 
                        msg_data[0] = (unsigned char)(window*16 + packet);
                        packet_count++;
                        if((packet_count == total_packets) || (packet == 5))
							{
                                service = REQUEST;
								packet = 6;   // ensures exit from packet loop

                            }
						else
							{
								service = UNACKD;
							}							
      					if (!lon_write_msg (ldv_handle, niTQ, service, 0x3E, msg_data, k, addr_mode, addr_data))
							{ 

								printf ("LON write Unsucessful. Application aborting.\n");
								return FALSE;      
                    		}
						else//clear_network_buffer(ldv_handle);
        
							{
                                if(packet!=6)					            
								    printf("LON packet [%d] write Succcesful.\n", packet);
								else 
									printf("LON packet [final] write Sucessful.\n");
                                //clear_network_buffer(ldv_handle);
					        }  
					}
			}

		goto done;
        //clear_network_buffer(ldv_handle);
        //  After last packet, we expect a response
        printf("Let's read ftp packet transfer responses - Expecting %d.\n",total_packets);
        for(i=0; i<=total_packets+3;i++)    //  CHANGED AEK
		{
			memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
        	done = FALSE;
        	count = 0;

    		while(!done & count < 750)
			{
			    if ((ldv_status = ldv_read (ldv_handle, &ldv_msg, sizeof(ExpAppBuffer))) == LDV_OK)
					{
						//  A message was received, Check to see if NV response                        
						if(TRUE)
							{			
		   	                    printf("FTP response packet [%d] received.\n",i);				
								done = TRUE;  
		                        display_packet((char *)&ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr));
							}
                        //  Something else was received, Retry
                        else count++; 	                                       
					}

				
				else
					{
        	          	if(ldv_status == LDV_NO_MSG_AVAIL) count++;
					}
			}
         }
       
done:
        clear_network_buffer(ldv_handle);
        printf("Data transfer to sign complete.\n Press Enter to Continue:\n\n");
		//getchar();

		//----------------------------------------------------------------------
        //                         PART III - FTP Conclusion 
		//-----------------------------------------------------------------------

        printf("Ready to send 'SNVT_file_req' to CLOSE transfer.\n\n");
 
        //  Conclude ftp transfer by sending SNVT_file_req (12 Bytes) to CLOSE       
	    for(i=0; i<=35; i++) msg_data[i] = 0x00;
	    msg_data[0] = 0x02;
	    msg_data[1] = 0x00;
		msg_data[2] = 0x03;
	    msg_data[3] = 0x01;
	    msg_data[4] = 0xF4;
	    msg_data[5] = 0x00;

	    if(!update_nv(ldv_handle, 0, msg_data, 12))
			{

				printf ("SNVT_file_req NV Unsucessful.\n");
				return FALSE;

			}
return;
        //printf("We are now waiting for 'SNVT_file_status' response from node.\nPress RETURN to Continue.\n");
        //getchar();    
        // Wait for Network Variable response, SVNT_file_status, from Node 
 		memset (&ldv_msg, 0, sizeof(ExpAppBuffer));     
        done = FALSE;
        count = 0;
    	while(!done & count < 750)

			{
			    if ((ldv_status = ldv_read (ldv_handle, &ldv_msg, sizeof(ExpAppBuffer))) == LDV_OK)
					{
						//  A message was received, Check to see if NV response                        
						if(ldv_msg.data.unv.must_be_one == 0x01)
							{			
		   	                    printf("Network Variable SNVT_file_status Response received.\n");				
								done = TRUE;  
		                        display_packet((char *)&ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr));
							}
                        //  Something else was received, Retry
                        else count++; 	                                       
					}
				else
					{
        	          	if(ldv_status == LDV_NO_MSG_AVAIL) count++;
					}
			}


		//  We didn't received the SNVT_file_status Response, we can't continue  		
		if(!done) 
            {
	          printf("Network Variable SNVT_file_status Response NOT received.\n");
              return FALSE;
			}       

		return TRUE;
	}


//******************************************************************************
//
//    Function: int lon_write_msg(void)
//
//    Description:
//
//
//
//******************************************************************************
int lon_write_msg (LDV_HANDLE ldv_handle, unsigned int queue, unsigned char service, 
					unsigned char function_code, unsigned char data[], int data_size,
					unsigned char addr_mode, unsigned char addr_data[])
  { 
	LDV_STATUS ldv_status;	
	ExpAppBuffer ldv_msg;
	
 	memset (&ldv_msg, 0, sizeof(ExpAppBuffer));
 	assemble_packet(&ldv_msg, queue, service, function_code, data, data_size, addr_mode, addr_data);
    display_packet((char *)&ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr));


    //  Application Header (2 Bytes)
	//ldv_msg.ni_hdr.q.queue = niTQ;
	//ldv_msg.ni_hdr.q.q_cmd = niCOMM;
    //ldv_msg.ni_hdr.q.length = sizeof (MsgHdr) + sizeof(ExplicitAddr) + (data_size + LON_DATA_CODE_SZ);

	//  Message Header (3 Bytes)
    // use explicit addressing and use the xxxxx service t
	//ldv_msg.msg_hdr.exp.addr_mode = LON_ADDR_MODE_EXPLICIT;
	//ldv_msg.msg_hdr.exp.st = UNACKD;
    //ldv_msg.msg_hdr.exp.length = (data_size + LON_DATA_CODE_SZ);
		
    // Network Address Header (11 Bytes)	 	
	// send the message with some arbitrary values set for retry, rpt_timer, tx_timer.
    //ldv_msg.addr.snd.sn.type = NI_SUBNET_NODE;
    //ldv_msg.addr.snd.sn.node = 3;
    //ldv_msg.addr.snd.sn.subnet = 1;
    //ldv_msg.addr.snd.sn.domain = 0;
	//ldv_msg.addr.snd.sn.retry = 3;
	//ldv_msg.addr.snd.sn.rpt_timer = 3;
	//ldv_msg.addr.snd.sn.tx_timer = 9;

    // Contruct the Message Data section of the packet(Size varies)
	//ldv_msg.data.exp.code = function_code;
	//memcpy (ldv_msg.data.exp.data, data, data_size * sizeof(Byte));

	if ((ldv_status = ldv_write (ldv_handle, &ldv_msg, ldv_msg.ni_hdr.q.length + sizeof(NI_Hdr))) != LDV_OK) {
		
		printf ("Error: Lonworks message send failed with status code %d\n", ldv_status);
		return (FALSE);
	}

	
	return (TRUE);
}

//******************************************************************************
//
//    Function: bool assemble_packet(ExpAppBuffer* ldv_msg, in data_size, int service)
//
//    Description:
//
//
//
//******************************************************************************
bool assemble_packet(ExpAppBuffer* ldv_msg, unsigned int queue, unsigned char service, 
                    unsigned char function_code, unsigned char data[], int data_size,
                    unsigned char addr_mode, unsigned char addr_data[]) 
	{
        int i;        
	    //  Application Header (2 Bytes)
		ldv_msg->ni_hdr.q.queue = queue;   // niTQ or niNTQ ?
		ldv_msg->ni_hdr.q.q_cmd = niCOMM;  // niNETMGMT;  //niCOMM;  
	    ldv_msg->ni_hdr.q.length = sizeof (MsgHdr) + sizeof(ExplicitAddr) + (data_size + LON_DATA_CODE_SZ);


		//  Message Header (3 Bytes)	
	    // use explicit addressing and use the xxxxx service t

	    ldv_msg->msg_hdr.exp.auth = 0;
		ldv_msg->msg_hdr.exp.st = service; 
    	ldv_msg->msg_hdr.exp.msg_type = 0;

  		ldv_msg->msg_hdr.exp.addr_mode = LON_ADDR_MODE_EXPLICIT;
		ldv_msg->msg_hdr.exp.length = (data_size + LON_DATA_CODE_SZ);

    	// Network Address Header (11 Bytldv_msg[09] -> 01
		// send the message with some arbitrary values set for retry, rpt_timer, tx_timer.

        if(g_address_mode==NI_SUBNET_NODE)
		{
    		// subnet node addressing 
    		ldv_msg->addr.snd.sn.type = NI_SUBNET_NODE;
    		ldv_msg->addr.snd.sn.node = g_node;
    		ldv_msg->addr.snd.sn.subnet = g_subnet; 
    		ldv_msg->addr.snd.sn.domain = g_domain;
			ldv_msg->addr.snd.sn.retry = 3;
			ldv_msg->addr.snd.sn.rpt_timer = 3;
			ldv_msg->addr.snd.sn.tx_timer = 9;
		}		
		else if(g_address_mode==NI_NEURON_ID)
		{	
        	//  Neuron ID Addressing 
    		ldv_msg->addr.snd.id.type = NI_NEURON_ID;
   		 	ldv_msg->addr.snd.id.domain = 0;	
			ldv_msg->addr.snd.id.retry = 3;
			ldv_msg->addr.snd.id.rpt_timer = 3;
			ldv_msg->addr.snd.id.tx_timer = 9;        
    		ldv_msg->addr.snd.id.subnet = 0;
        	for(i=0;i<6;i++) ldv_msg->addr.snd.id.nid[i] = neuron_id[i];
		}	

    	// Contruct the Message Data section of the packet (Size varies)
		ldv_msg->data.exp.code = function_code;
		memcpy (ldv_msg->data.exp.data, data, data_size * sizeof(Byte));

    	return (TRUE);
	}


//******************************************************************************
//

//    Function: bool display_packet(unsigned char *ptr, int msg_length)
//

//    Description:
// 
//ldv_msg[09] -> 01

//
//******************************************************************************
bool display_packet(unsigned char *ptr, int msg_len) 
	{
	    int i;
	    char highbyte, lowbyte;

	    for (i=0;i<msg_len; i++) 
			{
		      lowbyte=(0x0F & (*ptr));
		      highbyte=(0xF0 & (*ptr))>>4;
		      printf("ldv_msg[%02d] -> %1x%1x\n",i,highbyte,lowbyte);
		      ptr++;
		    }
	   return (TRUE);  
	}


//******************************************************************************
//

//    Function: lon_read_msgs (LVV_HANDLE ldv_handle)
//
//    Description:

// 
//
//

//******************************************************************************
int lon_read_msgs (LDV_HANDLE ldv_handle, ExpAppBuffer* ldv_msg)
 {
	LDV_STATUS ldv_status = LDV_NO_MSG_AVAIL; 	
	//ExpAppBuffer ldv_msg;
    int i;
    boolean done=FALSE;     
	
    //  LDV_OK = 0
    //  LDV_NO_MSG_AVAIL = 6
    //  LDVX_READ_FAILED = 14

    printf("Entering function lon_read_msgs\n"); 
   

	memset (ldv_msg, 0, sizeof(ExpAppBuffer));
    //  Wait for a message to arrive, keep looping until done
    while(!done) {

        //printf("  ldv status -> %d", ldv_status); 
	    if ((ldv_status = ldv_read (ldv_handle, ldv_msg, sizeof(ExpAppBuffer))) == LDV_OK)
          {
            //  LDV_OK, Message was read from network interface
    	    switch (ldv_msg->ni_hdr.q.queue) 
             {
				case niINCOMING: 
				{
					done=FALSE;
                    printf ("Received an Unsolicitated Incoming Lonworks Message Packet.\n");
                    display_packet((char *)ldv_msg, ldv_msg->ni_hdr.q.length + sizeof(NI_Hdr));
					break;
				}													
				case niRESPONSE: 
				{
					if (ldv_msg->msg_hdr.exp.cmpl_code == MSG_SUCCEEDS) 
                     {
     		    	   printf ("Received a MSG_SUCCEEDS completion event for sent message.\n");
				     } 
                   else if (ldv_msg->msg_hdr.exp.cmpl_code == MSG_NOT_COMPL)
                     {
    				   printf ("Received a MSG_NOT_COMPL completion event for sent message.\n");
				     }
                    else if (ldv_msg->msg_hdr.exp.cmpl_code == MSG_FAILS)
					 {
    				   printf ("Received a MSG_FAILS completion event for sent message.\n");
                     }

                    display_packet((char *)ldv_msg, ldv_msg->ni_hdr.q.length + sizeof(NI_Hdr));
                    done = TRUE;  
				    break;
				}
				
				default: 
				{
     					printf ("Received message with q.queue of %d\n", ldv_msg->ni_hdr.q.queue);
                	    done=FALSE;
				}
		     }  // end of switch clause
          }  // end of if clause
      }   // end of while clause  
 
	//if ( ldv_status != LDV_NO_MSG_AVAIL ) {
	//			
	//	printf ("Error: failure calling ldv_read(...), status: %d\n", ldv_status);
	//	done = FALSE;
	// }
	
	return done;
	}

//****************************************************************************
//
//    Function: lon_read_msgs (LVV_HANDLE ldv_handle)
//
//    Description:  This function closed the Lonworks interface
// 
//******************************************************************************
bool lon_close (LDV_HANDLE ldv_handle) 
	{
		LDV_STATUS ldv_status;
	
		if ((ldv_status = ldv_close (ldv_handle)) != LDV_OK) 
		    {
				printf ("Error: Unable to close U20, status code %d\n", ldv_status);
				return (FALSE);
			}
		
		return (TRUE);
	}





