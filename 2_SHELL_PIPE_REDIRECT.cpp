// edited from https://burweisnote.blogspot.com/2017/10/pipe.html
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <signal.h>
#include <vector>
#include <algorithm>
#include <sys/wait.h>
#include <fcntl.h>
// #include <sys/types.h>
// #include <sys/stat.h>

using namespace std;

#define MAX_PARAMS_IN_LINE 9487
#define BUFFER_SIZE 48763


int main()
{

	while (true)
	{
		// var
		int p1[2]; // pipe 1
		int p2[2]; // pipe 2
		int this_vbar = 0;  // this "|" pos (at the end of param)
		int last_vbar = 0;  // last "|" pos (at the first of param)
		int stdin_copy = dup(STDIN_FILENO);  // stdin fd
		int stdout_copy = dup(STDOUT_FILENO);  // stdout fd
		char pipe_buffer[BUFFER_SIZE] = {0};
		pid_t this_fork;
		string str_input = ""; // user input
		string str_temp = "";
		vector<string> cmd; // {"ls", "|", "head", "-1"}
		vector<int> vbar_pos; // vbars
		stringstream ss;


		// input
		cout<<"ʕ•́ᴥ•̀ʔっ";
		getline(cin, str_input, '\n');

		// parse
		if(str_input == "exit") // exit
		{
			cout<<"Bye bye!"<<endl;
			exit(0);
		}
		ss.str(str_input); // set to string stream
		vbar_pos.push_back(-1);
		for (int i = 0; ss >> str_temp; i++) // {"ls", "|", "head", "-1", ">", output}
		{
			cmd.push_back(str_temp); // store splited strings in vector cmd
			// cout<<str_temp<<endl;
			if (cmd.at(i) == "|")
			{
				vbar_pos.push_back(i); // store the pos of "|" in vector vbar_pos
			}
		}
		ss.clear();

		// pipe task
		for (int i = 0; i < cmd.size(); i++) // rec cmd
		{
			if (cmd.at(i) == "|" || i == cmd.size() - 1) // still have params!
			{
				if (cmd.at(i) == "|") // vbar: not last param => 
				{
					this_vbar = find(vbar_pos.begin(), vbar_pos.end(), i) - vbar_pos.begin();
					last_vbar = this_vbar - 1; // this_vbar, last_vbar are indexes in vbar_pos
				}

				// create pipes
				if (pipe(p1) < 0)
					perror("BAD PIPE 1!!!");
				if (pipe(p2) < 0)
					perror("BAD PIPE 2!!!");

				// exexute
				this_fork = fork();
				if(this_fork < 0)
				{
					perror("BAD FORK!!!");
				}
				if (this_fork == 0) // child fork : call execvp if cmd.at(i) is "|" or is the last word
				{
					if (i == cmd.size() - 1) // last
					{
						close(p1[1]);  // close pipe1 write
						dup2(p1[0], STDIN_FILENO); // attach p1in to stdin
						close(p1[0]);

						close(p2[0]); // close pipe2
						close(p2[1]);
					
						dup2(stdout_copy, STDOUT_FILENO); // attach to stdout

						// split commands to param
						char *param[MAX_PARAMS_IN_LINE] = {0}; // {"ls", "-al", ">", "ootput"}
						char *tmp;
						for (int j = 0; j < (i - vbar_pos.at(this_vbar)); j++)
						{
							tmp = strdup(cmd.at(vbar_pos.at(this_vbar) + j + 1).c_str());

							// handle redirection
							if(*tmp == '>')
							{
								j++;
								tmp = strdup(cmd.at(vbar_pos.at(this_vbar) + j + 1).c_str());	
								if(fork() == 0) // lemme try Execute
								{
									int fd_out2 = creat(tmp, 0644); // -rw-r--r--
									dup2(fd_out2, STDOUT_FILENO);
									close(fd_out2);
									execvp(param[0], param);
								}
							}
							else if(*tmp == '<')
							{
								j++;
								tmp = strdup(cmd.at(vbar_pos.at(this_vbar) + j + 1).c_str());
								if(fork() == 0)
								{
									int fd_in2 = open(tmp, O_RDONLY);
									dup2(fd_in2, STDIN_FILENO);
									close(fd_in2);
									execvp(param[0], param);
								}
								else
								{
									wait(NULL);
								}								
							}
							else // simple param: just add that
							{
								param[j] = tmp;
							}
						}					

						// param[i - vbar_pos.at(this_vbar)] = NULL; // last command

						execvp(param[0], param);
						// execve("/bin/"+param[0], param, NULL);

						// never come here
						exit(48763);
					}
					else
					{
						close(p1[1]);
						dup2(p1[0], STDIN_FILENO); // p1 will close after STDIN receive EOF
						close(p1[0]);

						close(p2[0]);
						dup2(p2[1], STDOUT_FILENO); // attach stdout to next pipe
						close(p2[1]);
						
						// split commands to param
						char *param[MAX_PARAMS_IN_LINE] = {0};
						for (int j = 0; j < (i - vbar_pos.at(last_vbar) - 1); j++) 
						{
							// strdup()會先用maolloc()配置與參數s字符串相同的空間大小，然後將參數s字符串的內容複製到該內存地址，
							// 然後把該地址返回。該地址最後可以利用free()來釋放。
							param[j] = strdup(cmd.at(vbar_pos.at(last_vbar) + j + 1).c_str()); 
						}

						param[i - vbar_pos.at(last_vbar) - 1] = NULL;
						execvp(param[0], param);

						exit(1);
					}
				}
				else
				{
					// parent process: pass old data to the new child and store new data
					close(p1[0]);
					dup2(p1[1], STDOUT_FILENO);
					close(p1[1]);

					cout << pipe_buffer << flush; // flush
					dup2(stdout_copy, STDOUT_FILENO); // p1 write end isn't used anymore, send EOF
					close(p2[1]);

					if (i < cmd.size() - 1)
					{
						memset(pipe_buffer, 0, sizeof(pipe_buffer));
						read(p2[0], pipe_buffer, sizeof(pipe_buffer));
					}

					close(p2[0]);
					
					// wait for exit
					int exit_code;
					waitpid(this_fork, &exit_code, 0);     
					if(exit_code != 0)
					{
						printf("★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★\n");
						// printf("Executed %s\n", params[0]);
						printf("(╯°□°）╯︵ ┻━┻ Process %d Terminated with exit code %d ┬──┬ ノ( ゜-゜ノ)\n", this_fork, exit_code);
						printf("☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆★☆\n");
					}
				}
			}
		}

		cout<<endl;
	}
}