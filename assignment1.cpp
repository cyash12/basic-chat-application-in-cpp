#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <math.h>

#include "../include/global.h"
#include "../include/logger.h"

#include <sstream>
#include <algorithm>
#include <string>

using namespace std;


string my_ip;
string my_hostname;
string my_port;
int sockfd;
string blank = " ";
struct addrinfo *myAddrInfo;
struct addrinfo hints;

//****reference*****

struct SockObj{
		  string hostname;
		  int num_msg_rcv;
		  string status;
		  string ip;
		  int cfd;
		  string port;
		  int num_msg_sent;
		  vector<string> blockeduser;
		  vector<string> msgbuffer;

		  bool operator<(const SockObj &rhs) const {
		    return atoi(port.c_str()) < atoi(rhs.port.c_str());
		  }
};
vector<SockObj> sock_list;


SockObj* newSockObj(int cfd, string hostname, string ip, string port) {
	  SockObj* hd = new SockObj;
	  hd->cfd = cfd;
	  hd->hostname = hostname;
	  hd->ip = ip;
	  hd->port = port;
	  hd->num_msg_sent = 0;
	  hd->num_msg_rcv = 0;
	  hd->status = "logged-in";
	  return hd;
}

SockObj* in_set_sock(string ip, string port) {
  for (unsigned int i = 0; i < sock_list.size(); ++i) {
    SockObj* hd = &sock_list[i];
    if (hd->ip == ip && hd->port == port) {
      return hd;
    }
  }
  return NULL;
}

SockObj* in_set_sock(string ip) {
  for (unsigned int i = 0; i < sock_list.size(); ++i) {
    SockObj* hd = &sock_list[i];
    if (hd->ip == ip) {
      return hd;
    }
  }
  return NULL;
}

SockObj* in_set_sock(int cfd) {
  for (unsigned int i = 0; i < sock_list.size(); ++i) {
    SockObj* hd = &sock_list[i];
    if (hd->cfd == cfd) {
      return hd;
    }
  }
  return NULL;
}

//tools functions***
void init_my_address(const char* port) { 

	  my_port = port;

	  char hostname[1024];
	  gethostname(hostname, sizeof(hostname) - 1);
	  my_hostname = hostname;

	  char buffer[256];
	  size_t buflen = 256;
	  //UDP
	  int sock = socket(AF_INET, SOCK_DGRAM, 0);
	  const char* google_dns = "8.8.8.8";
	  uint16_t dns_port = 53;
	  struct sockaddr_in serv;
	  memset(&serv, 0, sizeof(serv));
	  serv.sin_family = AF_INET;
	  serv.sin_addr.s_addr = inet_addr(google_dns);
	  serv.sin_port = htons(dns_port);
	  int err = connect(sock, (const sockaddr*)&serv, sizeof(serv));
	  sockaddr_in name;
	  socklen_t namelen = sizeof(name);
	  err = getsockname(sock, (sockaddr*)&name, &namelen);
	  my_ip = inet_ntop(AF_INET, &name.sin_addr, buffer, buflen);
	  close(sock);

	  int rv;
	  memset(&hints, 0, sizeof hints);  
	  hints.ai_family = AF_INET;        
	  hints.ai_socktype = SOCK_STREAM;  
	  hints.ai_flags = AI_PASSIVE;      

	    if ((rv = getaddrinfo(NULL, (const char*)my_port.c_str(), &hints, &myAddrInfo)) != 0) {
    		exit(1);
  		}
  		sockfd = socket(myAddrInfo->ai_family, myAddrInfo->ai_socktype, myAddrInfo->ai_protocol);
  		bind(sockfd, myAddrInfo->ai_addr, myAddrInfo->ai_addrlen);
  		freeaddrinfo(myAddrInfo);
}
void split_msg(string& src, const string& separator, vector<string>& dest){
    string sub_str;
	string str = src;
    string::size_type start = 0, index;
    dest.clear();
    index = str.find_first_of(separator,start);
    do
    {
        if (index != string::npos)
        {    
            sub_str = str.substr(start,index-start );
            dest.push_back(sub_str);
            start =index+separator.size();
            index = str.find(separator,start);
            if (start == string::npos) break;
        }
    }while(index != string::npos);

    sub_str = str.substr(start);
    dest.push_back(sub_str);
}

int char_to_int(char s) {
	if (s == '1') return 1;
	if (s == '2') return 2;
	if (s == '3') return 3;
	if (s == '4') return 4;
	if (s == '5') return 5;
	if (s == '6') return 6;
	if (s == '7') return 7;
	if (s == '8') return 8;
	if (s == '9') return 9;
	return 0;
}

int str_to_int(string str) {
	int len = str.length();
	int res = 0;
	for (int i = len - 1; i >= 0; --i) {
		res += char_to_int(str[i]) * pow(10.0, double(len - i - 1));
	}
	return res;
}


bool valid_ip(string ip_test) {
	int dot_num = 0;
	for(int i = 0; i < ip_test.length(); ++i){
		if(ip_test[i] == '.') {
			dot_num++;
		}
	}
	if(dot_num != 3) return false;
	vector<string> ip_parts;
	split_msg(ip_test,".", ip_parts);
	for(int i = 0; i < 4; ++i){
		for(int j = 0; j < ip_parts[i].length(); ++j){
			if(ip_parts[i][j] > '9' || ip_parts[i][j] < '0') return false;
		}
	}
	for(int i = 0; i < 4; ++i){
		if(str_to_int(ip_parts[i]) > 255) return false;
	}
	return true;
}





//***logger***
void author_logger() {
	const char* command = "AUTHOR";
	cse4589_print_and_log("[%s:SUCCESS]\n", command);
	string ubit_name = "yashprak";
	cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n",  ubit_name.c_str());
	cse4589_print_and_log("[%s:END]\n", command);
}
void error_logger(string cmd) {
  cse4589_print_and_log("[%s:ERROR]\n", cmd.c_str());
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void IP_logger(){
	const char* command = "IP";
	cse4589_print_and_log("[%s:SUCCESS]\n", command);
	cse4589_print_and_log("IP:%s\n", my_ip.c_str());
	cse4589_print_and_log("[%s:END]\n", command);
}
void event_logger(string client_ip, string msg) {
	const char* command = "RECEIVED";
	cse4589_print_and_log("[%s:SUCCESS]\n", command);
	cse4589_print_and_log("msg from:%s\n[msg]:%s\n", client_ip.c_str(), msg.c_str());
	cse4589_print_and_log("[%s:END]\n", command);
}
void events_logger(string from_ip, string msg, string to_ip) {
	const char* command = "RELAYED";
	cse4589_print_and_log("[%s:SUCCESS]\n", command);
	cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", (const char*)from_ip.c_str(), (const char*)to_ip.c_str(), (const char*)msg.c_str());
	cse4589_print_and_log("[%s:END]\n", command);
}
void port_logger() {
	const char* command = "PORT";
	cse4589_print_and_log("[%s:SUCCESS]\n", command);
	cse4589_print_and_log("PORT:%d\n", str_to_int(my_port));
	cse4589_print_and_log("[%s:END]\n", command);
}
void list_logger() {
	string cmd = "LIST";
	cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
	sort(sock_list.begin(), sock_list.end());
  	for (unsigned int i = 0; i < sock_list.size(); ++i) {
    	cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i + 1,
                          sock_list[i].hostname.c_str(),
                          sock_list[i].ip.c_str(), str_to_int(sock_list[i].port));
  }
  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void blocked_logger(string cli_ip) {
		string cmd = "BLOCKED";
	    if (!valid_ip(cli_ip) < 0 || in_set_sock(cli_ip) == NULL) {
	    	error_logger(cmd);
	    	return;
		  }
		  SockObj* hd = in_set_sock(cli_ip);

		  cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
		  for (int i = 0; i < hd->blockeduser.size(); ++i) {
		    SockObj* new_hd = in_set_sock(hd->blockeduser[i]);
		    cse4589_print_and_log("%-5d%-35s%-20s%-8s\n", i + 1, new_hd->hostname.c_str(),
		                          new_hd->ip.c_str(), new_hd->port.c_str());
		  }
		  cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void stat_logger() {
	string command = "STATISTICS";
    cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
    sort(sock_list.begin(), sock_list.end());
  	for (unsigned int i = 0; i < sock_list.size(); ++i) {
    cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i + 1,
                          sock_list[i].hostname.c_str(),
                          sock_list[i].num_msg_sent, sock_list[i].num_msg_rcv,
                          sock_list[i].status.c_str());
  }
  cse4589_print_and_log("[%s:END]\n", command.c_str());
}

//***CLIENT***

int start_client(string my_port) {
  // ****init****
  const char* charPORT = my_port.c_str();
  init_my_address(charPORT);

  // variables
 
  fd_set readfds;   
  int fdmax = sockfd;
  bool login_st = 0;

  string msg="";
  char charmsg[65535];
  vector<string> msg_p;
  
  string myServerInfo[2];

  struct addrinfo *servinfo,*p2;


  // loop
	  while (1) {
	  		FD_ZERO(&readfds);
	  		memset(charmsg,0,sizeof charmsg);
			msg = "";


	  		if(login_st == 0){
	  			FD_SET(fileno(stdin), &readfds);
	  			select(1, &readfds, NULL, NULL, NULL);
	  			if (FD_ISSET(fileno(stdin), &readfds)) {
	  				read(fileno(stdin), charmsg, sizeof charmsg);
	  				msg = charmsg;
      				msg = msg.substr(0, msg.length() - 1);
      				split_msg(msg," ", msg_p);
      				fflush(stdin);

      					  int mark=0;
					      if(msg_p[0] == "LOGIN"){mark =1;}
					      if(msg_p[0] == "AUTHOR"){mark =2;}
					      if(msg_p[0] == "IP"){mark =3;}
					      if(msg_p[0] == "PORT"){mark =4;}
					      if(msg_p[0] == "EXIT"){mark =5;}


					      switch (mark){
					      		//*****login ip port
					      		case 1:{
								            sock_list.clear(); 
								            

											if(valid_ip(msg_p[1])){
														msg = "";
														string temp_num = "1 ";
														msg = temp_num + my_hostname + blank+ my_ip+ blank + my_port;
														if(myServerInfo[0] == msg_p[1] && myServerInfo[1] == msg_p[2])
														{
															char charmgs[65535];
															memset(charmgs,0,sizeof charmgs);
														
															string line = "";
															vector<string> mgs_p;
															vector<string> line_p;
															login_st = 1;

															send(sockfd, (const char*)msg.c_str(), msg.length(), 0);
															recv(sockfd, charmgs, sizeof charmgs, 0);

															line= charmgs;
															split_msg(line, "\n", line_p);

															for(int i = 0; i < line_p.size(); ++i){
																split_msg(line_p[i], " ", mgs_p);
																			switch (str_to_int(mgs_p[0])){
																			    	case 9:{	
																			    		//cout<< msg;	
																			    		msg = mgs_p[3];
																						for(int i = 4; i < mgs_p.size(); ++i){
																						      msg = msg +blank+ mgs_p[i];
																						}
																						//cout<< msg;
																						event_logger(mgs_p[1],msg);
																						break;
																			    	}
																			    	case 1:{
																			    		sock_list.clear();
																	    				for(int i =1; i < (mgs_p.size()-1); i += 3){
																				      			if(in_set_sock(mgs_p[i+1],mgs_p[i+2]) == NULL ){
																				               	sock_list.push_back(*newSockObj(-2, mgs_p[i], mgs_p[i + 1], mgs_p[i + 2]));
																				           		}
																				            }										         
																				          break;
																			    		
																			    	}
																			    	case 6:{
																			    		msg = mgs_p[2];
																						for(int i = 3; i < mgs_p.size(); ++i){
																							msg = msg +blank+ mgs_p[i];
																						}
																						event_logger(mgs_p[1], msg);
																			    	}

																			    }


															}


															cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());
															cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
															break;

														}else{
																myServerInfo[0] = msg_p[1]; 
																myServerInfo[1] = msg_p[2]; 
															}
														int rv;
														if ((rv = getaddrinfo(msg_p[1].c_str(), msg_p[2].c_str(), &hints, &servinfo)) != 0) {
															cse4589_print_and_log("[%s:ERROR]\n", msg_p[0].c_str());
															cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
															break;
														}
																for(p2 = servinfo; p2 != NULL; p2 = p2->ai_next) {
																	if (connect(sockfd, p2->ai_addr, p2->ai_addrlen) == -1) {
																		close(sockfd);
																		continue;
																	}
																	break;
																}
																if (p2 == NULL) {
																	cse4589_print_and_log("[%s:ERROR]\n", msg_p[0].c_str());
																	cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
																	break;
																}
													
																freeaddrinfo(servinfo); 
																login_st = 1;
																send(sockfd, (const char*)msg.c_str(), msg.length(), 0);
																cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());
																cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
																break;
															
											}else{cse4589_print_and_log("[%s:ERROR]\n", msg_p[0].c_str());
												  cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());break;}
							            } 
					      		//****print author
					      		case 2:{
								    author_logger();
			          				break;
					      		}
					      		//******print IP
					      		case 3:{
					      			IP_logger();
			          				break;
					      		}
					      		//*******print port
					      		case 4:{
									port_logger();
									break;
					      		}
					      		//*********EXIT
					      		case 5:{
									cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());
							        string temp_num = "5 ";
							        msg = temp_num + my_ip;
							        send(sockfd, (const char*)msg.c_str(), msg.length(), 0); 
							        cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
							        exit(0);
							        break;
					      		}
					      }
	  			}else{continue;}
	  		}else{
	  			FD_SET(fileno(stdin), &readfds);
	  			FD_SET(sockfd, &readfds);
	  			fdmax = sockfd;
	  			select(fdmax + 1, &readfds, NULL, NULL, NULL);
		  				if (FD_ISSET(fileno(stdin), &readfds)) {
							  read(fileno(stdin), charmsg, sizeof charmsg);
						      msg = charmsg;
						      fflush(stdin);
						      msg = msg.substr(0, msg.length() - 1);
						      split_msg(msg," ", msg_p);

							      int mark=0;
							      if(msg_p[0] == "LOGOUT"){mark =1;}
							      if(msg_p[0] == "EXIT"){mark =2;}
							      if(msg_p[0] == "IP"){mark =3;}
							      if(msg_p[0] == "LIST"){mark =4;}
							      if(msg_p[0] == "AUTHOR"){mark =5;}
							      if(msg_p[0] == "PORT"){mark =6;}
							      if(msg_p[0] == "REFRESH"){mark =7;}
							      if(msg_p[0] == "BROADCAST"){mark =8;}
							      if(msg_p[0] == "BLOCK"){mark =9;}
							      if(msg_p[0] == "UNBLOCK"){mark =10;}
							      if(msg_p[0] == "SEND"){mark =11;}

							      switch(mark){
							      		case 1:{//CMD***LOGOUT
							      				cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());
										        string temp_num = "1 ";
										        msg = temp_num + my_ip;
										        send(sockfd, (const char*)msg.c_str(), msg.length(), 0);
                                                //close(sockfd);
                                                login_st = 0;
										        cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
										        break;
							      		}
							      		case 2:{
							      				cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());
										        string temp_num = "2 ";
										        msg = temp_num +my_ip;
										        send(sockfd, (const char*)msg.c_str(), msg.length(), 0);
										        cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
										    	exit(0);
										        break;
							      		}
							      		case 3:{
							      			IP_logger();
								         	 break;
							      		}
							      		case 4:{
							      		  list_logger();
								          break;
										}
							      		case 5:{
							      		author_logger();
								          break;
										}
							      		case 6:{  
							      		port_logger();
								          break;
										}
							      		case 7:{
							      		    cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());
									        msg = "7";
									        msg = msg + blank +my_ip;
									        send(sockfd, (const char*)msg.c_str(), msg.length(), 0); 
									        cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
									        break;
							      		}
							      		case 8:{
							      			cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());
									        string temp_num = "6";
									        msg = msg_p[1];
									        for(int i = 2; i < msg_p.size();i++){
									        	msg += blank + msg_p[i];
									   		}
									   	    msg = temp_num + blank + my_ip + blank + msg;
									        send(sockfd, (const char*)msg.c_str(), msg.length(), 0);
									        cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
									        break;							      			
							      		}
							      		case 9:{
							      			// cout << msg_p[1];
							      			if (in_set_sock(msg_p[1])==NULL || !valid_ip(msg_p[1])){
							      				cse4589_print_and_log("[%s:ERROR]\n", msg_p[0].c_str());	
							      				cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
							      				break;
										    }
										    // SockObj* hd = in_set_sock(sockfd);
										    SockObj *hd = in_set_sock(my_ip);
										    if (hd == NULL ){
										    	cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());								      		  
									            string temp_num = "2 ";
									            msg = temp_num + my_ip + blank + msg_p[1];
									            send(sockfd, (const char*)msg.c_str(), msg.length(), 0);
									            cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
									            break;
										    }
											vector<string>::iterator ret;
											ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), msg_p[1]);
											if(ret != hd->blockeduser.end()){break;}
											hd->blockeduser.push_back(msg_p[1]);
									      		cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());								      		  
									            string temp_num = "2 ";
									            msg = temp_num + my_ip + blank + msg_p[1];
									            send(sockfd, (const char*)msg.c_str(), msg.length(), 0);
									            cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
									            break;
							      		}
							      		case 10:{
							      			if (in_set_sock(msg_p[1])==NULL){
							      				cse4589_print_and_log("[%s:ERROR]\n", msg_p[0].c_str());	
							      				cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
							      				break;
							      			}
							      			SockObj* hd = in_set_sock(sockfd);
							      			if(hd == NULL){
							      				  string temp_num = "3 ";
										          msg = temp_num + my_ip + blank + msg_p[1];
										          send(sockfd, (const char*)msg.c_str(), msg.length(), 0);
										          cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());
										          cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
										          break;

							      			}else{
											vector<string>::iterator ret;
											ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), msg_p[1]);
											if(ret == hd->blockeduser.end()){break;}
										          string temp_num = "3 ";
										          msg = temp_num + my_ip + blank + msg_p[1];
										          send(sockfd, (const char*)msg.c_str(), msg.length(), 0);
										          cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());
										          cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());
										          break;
										    }
							      		}
							      		case 11:{
										        string to_ip = msg_p[1];
										       // cout<< msg;
										        string err ="SEND";
										        if(!valid_ip(to_ip) || in_set_sock(to_ip) == NULL){
										        	error_logger(err);
										        }else{
										        	string msg = "9 ";
										     		msg = msg + my_ip + blank + to_ip;
											        for(int i = 2; i < msg_p.size(); ++i){
											          msg = msg +blank+ msg_p[i];
											        }
											        //cout<< msg;
											        send(sockfd, (const char*)msg.c_str(), msg.length(), 0);
											        cse4589_print_and_log("[%s:SUCCESS]\n", msg_p[0].c_str());	
											        cse4589_print_and_log("[%s:END]\n", msg_p[0].c_str());	
										        }
										        break;
							      		}
							      }



		  				}else if (FD_ISSET(sockfd, &readfds)) {
		  					if(recv(sockfd, charmsg, sizeof charmsg, 0) == 0){
						        close(sockfd);
						        sockfd = 0;
						        login_st = 0; 
						     }else{
						     	msg = charmsg;
							    split_msg(msg," ", msg_p);

							    switch (str_to_int(msg_p[0])){
							    	case 9:{	
							    		//cout<< msg;	
							    		msg = msg_p[3];
										for(int i = 4; i < msg_p.size(); ++i){
										      msg = msg +blank+ msg_p[i];
										}
										//cout<< msg;
										event_logger(msg_p[1],msg);
										break;
							    	}
							    	case 1:{
							    		sock_list.clear();
					    				for(int i =1; i < (msg_p.size()-1); i += 3){
								      			if(in_set_sock(msg_p[i+1],msg_p[i+2]) == NULL ){
								               	sock_list.push_back(*newSockObj(-2, msg_p[i], msg_p[i + 1], msg_p[i + 2]));
								           		}
								            }										         
								          break;
							    		
							    	}
							    	case 6:{
							    		msg = msg_p[2];
										for(int i = 3; i < msg_p.size(); ++i){
											msg = msg +blank+ msg_p[i];
										}
										event_logger(msg_p[1], msg);
							    	}

							    }
						     }
		  				}
	  		}

	  }
}

//***SERVER***

int start_server(string my_port) {

  // init
  const char* charPORT = my_port.c_str();
  init_my_address(charPORT);
  int yes=1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  listen(sockfd, 256);

  // variables
  struct sockaddr_storage remoteaddr;        
  socklen_t addrlen = sizeof remoteaddr; 

  char charmsg[65535]; 
  string msg;
  vector<string> msg_p;
  struct addrinfo *ai, *p;   


  fd_set readfds;                            
  int fdtemp;                                   
  int masterlist[256] = {0};
  int fdmax = sockfd;



  // core loop
  while (1) {
    // init fd
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    FD_SET(sockfd, &readfds);

    for (int i = 0; i < 256; i++) { // add child sockets to set
	      int sd = masterlist[i];
	      if (sd > 0) FD_SET(sd, &readfds);
	      if (sd > fdmax) fdmax = sd;
    }
    memset(&charmsg[0], 0, sizeof(charmsg));
    msg = "";

    select(fdmax + 1, &readfds, NULL, NULL, NULL);

    if (FD_ISSET(fileno(stdin), &readfds)) {
      read(fileno(stdin), charmsg, sizeof charmsg);
      fflush(stdin);
      msg = charmsg;
      msg = msg.substr(0, msg.length() - 1);
  	  split_msg(msg," ", msg_p); 
  	  		  int mark;
		      if(msg_p[0] == "PORT"){mark =1;}
		      if(msg_p[0] == "BLOCKED"){mark =2;}
		      if(msg_p[0] == "LIST"){mark =3;}
		      if(msg_p[0] == "STATISTICS"){mark =4;}
		      if(msg_p[0] == "IP"){mark =5;}
		      if(msg_p[0] == "AUTHOR"){mark = 6;}
		    switch (mark){
    		  	case 1:{
					port_logger();
					break;
				}
				case 2:{
					blocked_logger(msg_p[1]);
					break;
				}
				case 3:{
					list_logger();
					break;
				}
     		  	case 4: {
					stat_logger();
					break;
				   }
				case 5:{
					IP_logger();
      			    break;
				}     
				 
				case 6:{
					author_logger();
     		  		break;
				}
    		}
    }else if (FD_ISSET(sockfd, &readfds)) {
      fdtemp = accept(sockfd, (struct sockaddr*)&remoteaddr, &addrlen);
      for (int i = 0; i < 256; i++) { 
    			if (masterlist[i] == 0 ){
    				masterlist[i] = fdtemp;
    				break;
    			}
      }
      
      recv(fdtemp, charmsg, sizeof charmsg,0);
      
      msg = charmsg;
      split_msg(msg," ", msg_p);

      switch (str_to_int(msg_p[0])){
      			case 1:{
					string host = msg_p[1];
					string host_ip = msg_p[2];
					string port = msg_p[3];
					SockObj* hd = in_set_sock(fdtemp);

					if(hd == NULL) {
						hd = newSockObj(fdtemp, host, host_ip, port);
						sock_list.push_back(*hd);
					} else{
						hd->status = "logged-in";
						if(!hd->msgbuffer.empty()){
							string mgs = "";
							string hu = "/n";
							for(vector<string>::iterator it= hd->msgbuffer.begin(); it < hd->msgbuffer.end(); it++){
								mgs = mgs + hu +*it;
							}
							send(hd->cfd, (const char*)mgs.c_str(),mgs.length(),0);
							hd->msgbuffer.clear();
						}
					}
					string message = "1";
					for (unsigned int i = 0; i < sock_list.size(); ++i) {
				    	if (sock_list[i].status == "logged-in") {
				      		message = message +blank + sock_list[i].hostname + blank + sock_list[i].ip + blank+ sock_list[i].port;
				     }
					 }
				  
					  send(fdtemp, message.c_str(), strlen(message.c_str()), 0);
		         	   break;
      			}
      			case 2:{
					string from_ip = msg_p[1];
					string to_ip = msg_p[2];
					SockObj *hd = in_set_sock(from_ip);
					if(!valid_ip(to_ip) || in_set_sock(to_ip) == NULL) {
						error_logger("BLOCK");
						break;
					}
					vector<string> ::iterator ret;
					ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip);
					if(ret == hd->blockeduser.end()){
						hd->blockeduser.push_back(to_ip);
						break;
					}else{
						error_logger("BLOCK");
						break;
					}
		            break;
      			}
      			case 3:{
					string from_ip = msg_p[1];
					string to_ip = msg_p[2];
					SockObj *hd = in_set_sock(from_ip);
					if(!valid_ip(to_ip) || in_set_sock(to_ip) == NULL) {
						//error_logger("UNBLOCK");
						break;
					}
					vector<string> ::iterator ret;
					ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip);
					if(ret == hd->blockeduser.end()){
						error_logger("UNBLOCK");
						break;
					}
					else{
						hd->blockeduser.erase(ret);
						break;
					}
		            break;
      			}
      			case 4:{
					string ip_addr = msg_p[1];
					SockObj* hd = in_set_sock(ip_addr);
					if(hd != NULL){
						hd->status = "logged-out";
					}
					break;
      			}
      			case 5:{
					// if (in_set_sock(fdtemp) != NULL) {
					// 		sock_list.erase(sock_list.begin() + i--);
					// }
					for(int i = 0; i < sock_list.size(); ++i){
						if(sock_list[i].cfd == fdtemp){
							sock_list.erase(sock_list.begin() + i--);
						}
					}
					break;
      			}
      			case 6:{
      				    string from_ip = msg_p[1];
						SockObj* hd2 = in_set_sock(from_ip);
						string con = "255.255.255.255";
						if(hd2 == NULL){break;}
						for(int i = 0; i < sock_list.size(); i++){
							if(sock_list[i].ip == from_ip) continue;
							vector<string>::iterator ret;
							ret = find(hd2->blockeduser.begin(), hd2->blockeduser.end(), sock_list[i].ip);
							if(ret == hd2->blockeduser.end()){
								if(hd2->status == "logged-in"){
								send(hd2->cfd, (const char*)msg.c_str(), msg.length(), 0);
								sock_list[i].num_msg_rcv = sock_list[i].num_msg_rcv +1;
								hd2->num_msg_sent = hd2->num_msg_sent +1;
								string message;
								message = msg_p[2];
									for(int m = 3; m < msg_p.size(); m++){
										message = message +blank+ msg_p[m];
									}
								events_logger(from_ip, message, con);
							// }else{
							// 	sock_list[i].msgbuffer.push_back(msg);
							// 	sock_list[i].num_msg_rcv = sock_list[i].num_msg_rcv +1;
							// 	hd2->num_msg_sent = hd2->num_msg_sent +1;
							// 		string message;
							// 		message = msg_p[3];
							// 			for(int m = 4; m < msg_p.size(); m++){
							// 				message = message +blank+ msg_p[m];
							// 			}
							// 	events_logger(from_ip, message, con);
							// }
							}
						}

						  	break;
      			}
      		}
      			case 7:{
					string message = "1";
					for (unsigned int i = 0; i < sock_list.size(); ++i) {
				    	if (sock_list[i].status == "logged-in") {
				      		message += blank + sock_list[i].hostname + blank + sock_list[i].ip + blank +
				             sock_list[i].port;
				    	}
				  	}
				  	send(fdtemp, message.c_str(), strlen(message.c_str()), 0);
		            break;
      			}
				case 9:{
      				string from_ip = msg_p[1];
		        	string to_ip = msg_p[2];

					SockObj* hd = in_set_sock(to_ip);
					SockObj* hd2 = in_set_sock(from_ip);

					if(hd == NULL){
						break;
					}
					//cout<< msg;
						vector<string> ::iterator ret;
						ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), from_ip);
						if(ret == hd->blockeduser.end()){
							if(hd->status == "logged-in"){
//cout<< msg;
							send(hd->cfd, (const char*)msg.c_str(), msg.length(), 0);
							hd->num_msg_rcv = hd->num_msg_rcv +1;
							hd2->num_msg_sent = hd2->num_msg_sent +1;
							string message;
							message = msg_p[3];
								for(int m = 4; m < msg_p.size(); m++){
									message = message +blank+ msg_p[m];
								}
							events_logger(from_ip, message, to_ip);
						}else{
						hd->msgbuffer.push_back(msg);
						hd->num_msg_rcv = hd->num_msg_rcv +1;
						hd2->num_msg_sent = hd2->num_msg_sent +1;
							string message;
							message = msg_p[3];
								for(int m = 4; m < msg_p.size(); m++){
									message = message +blank+ msg_p[m];
								}
						events_logger(from_ip, message, to_ip);
					}
					break;
      				}
      			}
      }
  }else {
	     for (int i = 0; i < 256; i++) {
	        fdtemp = masterlist[i];
	        if (FD_ISSET(fdtemp, &readfds)) {
	          if (read(fdtemp, charmsg, sizeof charmsg) == 0) { // Check if closing
	            close(fdtemp);
	            masterlist[i] = 0;
	          } else {  // handle events
	            msg = charmsg;
	      split_msg(msg," ", msg_p);
	      switch (str_to_int(msg_p[0])){
	      			case 9:{
	      				string from_ip = msg_p[1];
			        	string to_ip = msg_p[2];
//cout<< msg;
						SockObj* hd = in_set_sock(to_ip);
						SockObj* hd2 = in_set_sock(from_ip);

						if(hd == NULL){
							break;
						}
							vector<string> ::iterator ret;
							ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), from_ip);
							if(ret == hd->blockeduser.end()){
								if(hd->status == "logged-in"){
									//cout<< msg;
								send(hd->cfd, (const char*)msg.c_str(),msg.length(), 0);
								hd->num_msg_rcv = hd->num_msg_rcv +1;
								hd2->num_msg_sent = hd2->num_msg_sent +1;
								string message;
								message = msg_p[3];
									for(int m = 4; m < msg_p.size(); m++){
										message = message +blank+ msg_p[m];
									}
								events_logger(from_ip, message, to_ip);
							}else{
							hd->msgbuffer.push_back(msg);
							hd->num_msg_rcv = hd->num_msg_rcv +1;
							hd2->num_msg_sent = hd2->num_msg_sent +1;
								string message;
								message = msg_p[3];
									for(int m = 4; m < msg_p.size(); m++){
										message = message +blank+ msg_p[m];
									}
							events_logger(from_ip, message, to_ip);
						}
						break;
	      				}
	      			}

	      			case 1:{
						string host = msg_p[1];
						string host_ip = msg_p[2];
						string port = msg_p[3];
						SockObj* hd = in_set_sock(fdtemp);

						if(hd == NULL) {
							hd = newSockObj(fdtemp, host, host_ip, port);
							sock_list.push_back(*hd);
						} else{
							hd->status = "logged-in";
							if(!hd->msgbuffer.empty()){
								for(vector<string>::iterator it= hd->msgbuffer.begin(); it < hd->msgbuffer.end(); it++){
									string mgs = *it;
									send(hd->cfd, (const char*)mgs.c_str(),mgs.length(),0);
								}
								hd->msgbuffer.clear();
							}
						}
						string message = "1";
						for (unsigned int i = 0; i < sock_list.size(); ++i) {
					    	if (sock_list[i].status == "logged-in") {
					      		message = message +blank + sock_list[i].hostname + blank + sock_list[i].ip + blank+ sock_list[i].port;
					     }
						 }
					  
						  send(fdtemp, message.c_str(), strlen(message.c_str()), 0);
			         	   break;
	      			}

	      			case 2:{
						string from_ip = msg_p[1];
						string to_ip = msg_p[2];
						SockObj *hd = in_set_sock(from_ip);
						if(!valid_ip(to_ip) || in_set_sock(to_ip) == NULL) {
							error_logger("BLOCK");
						}
						vector<string> ::iterator ret;
						ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip);
						if(ret == hd->blockeduser.end()){
							hd->blockeduser.push_back(to_ip);
						}
						else{
							error_logger("BLOCK");
						}
			            break;
	      			}

	      			case 3:{
						string from_ip = msg_p[1];
						string to_ip = msg_p[2];
						SockObj *hd = in_set_sock(from_ip);
						if(!valid_ip(to_ip) || in_set_sock(to_ip) == NULL) {
							error_logger("UNBLOCK");
						}
						vector<string> ::iterator ret;
						ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip);
						if(ret == hd->blockeduser.end()){
							error_logger("UNBLOCK");
						}
						else{
							hd->blockeduser.erase(ret);
						}
			            break;
	      			}

	      			case 4:{
						string ip_addr = msg_p[1];
						SockObj* hd = in_set_sock(ip_addr);
						if(hd != NULL){
							hd->status = "logged-out";
						}
						break;
	      			}

	      			case 5:{
	      				for (int i = 0; i < sock_list.size(); ++i) {
							if (sock_list[i].cfd == fdtemp) {
									sock_list.erase(sock_list.begin() + i--);	
							}
						}
						break;
	      			}

	      			case 6:{
	      				string from_ip = msg_p[1];
						SockObj* hd2 = in_set_sock(from_ip);
						if(hd2 == NULL){break;}
						string con = "255.255.255.255";

						for(int i = 0; i < sock_list.size(); ++i){
							if (sock_list[i].ip == from_ip){
								continue;
							}
							vector<string>::iterator ret;
							ret = find(sock_list[i].blockeduser.begin(), sock_list[i].blockeduser.end(), from_ip);
							if(ret == sock_list[i].blockeduser.end()){
								if(sock_list[i].status == "logged-in"){
								send(sock_list[i].cfd, (const char*)msg.c_str(), msg.length(), 0);
								sock_list[i].num_msg_rcv = sock_list[i].num_msg_rcv +1;
								hd2->num_msg_sent = hd2->num_msg_sent +1;
								string message;
								message = msg_p[2];
									for(int m = 3; m < msg_p.size(); m++){
										message = message +blank+ msg_p[m];
									}

								events_logger(from_ip, message, con);
								}
							}
						}

						  	break;
	      			}
	      			case 7:{
						string message = "1";
						for (unsigned int i = 0; i < sock_list.size(); ++i) {
					    	if (sock_list[i].status == "logged-in") {
					      		message += blank + sock_list[i].hostname + blank + sock_list[i].ip + blank +
					             sock_list[i].port;
					    	}
					  	}
					  	send(fdtemp, message.c_str(), strlen(message.c_str()), 0);
			            break;
	      			}
          }
        }
      }
    }
}
}

}






//****************main******************

int main(int argc, char **argv){
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
    fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	if (*argv[1] == 'c') {
		string port = argv[2];
    	start_client(port);
  	} else if (*argv[1] == 's') {
  		string port = argv[2];
    	start_server(port);
  	} else {
    	return 1;
  	}
 	return 0;
}
