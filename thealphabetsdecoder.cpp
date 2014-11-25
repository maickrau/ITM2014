/**
The test program for decoding alphabets in alphabets.txt
Same thing with the numbersdecoder
**/
#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>

int main (){

    std::bitset<167685> c; //Here put the original code length
    std::ifstream input("aBITisaBit.bin",std::ios::binary); //File that contains Huffman codes

    int j=0;
    for(int i=0;i<20961;i++){ //i increament to the size of file in bytes.
	std::bitset<8> abyte;
	unsigned long n;
	input.seekg(i,input.beg); //seek the ith byte
	input.read(reinterpret_cast<char*>(&n), 1) ; //read the ith byte
	abyte = n;
	for(int t=0; t<8; t++){

		c[j]=abyte[t]; 
		j++;
	}	

    } 

    std::vector<char> s; //Again, decoded data is here

 for(int i=0;i<167685;){
 //The following are a hard-coded Huffman tree in the form of "if(){}else{}" Not so adaptable and easy way for it can only be used for the Huffman code dictionary. But this can be short if eliminate spaces.
	if(c[i]==0){i++;
	
		if(c[i]==0){i++;
	
			if(c[i]==0){i++;

				if(c[i]==0){i++;
	
					if(c[i]==0){s.push_back('r');i++;
	
					}
					else{i++;

						if(c[i]==0){s.push_back('v');i++;
	
						}
						else{s.push_back('h');i++;
	
						}

					}

				}
				else{s.push_back('s');i++;
	
				}

			}
			else{i++;

				if(c[i]==0){s.push_back('n');i++;
	
				}
				else{s.push_back('u');i++;
	

				}

			}

		}
		else{i++;
	
			if(c[i]==0){i++;

				if(c[i]==0){s.push_back('e');i++;
	
				}
				else{s.push_back('o');i++;
	
				}

			}
			else{s.push_back('i');i++;
	
			}

		}

	}
	else{i++;

		if(c[i]==0){i++;
	
			if(c[i]==0){s.push_back('a');i++;
	
			}
			else{i++;

				if(c[i]==0){s.push_back('k');i++;
	
				}
				else{i++;

					if(c[i]==0){s.push_back('m');i++;
	
					}
					else{s.push_back('p');i++;
	
					}
	
				}
	
			}

		}
		else{i++;
	
			if(c[i]==0){i++;

				if(c[i]==0){s.push_back('l');i++;
	

				}
				else{i++;

					if(c[i]==0){i++;

						if(c[i]==0){i++;

							if(c[i]==0){s.push_back('d');i++;
	
							}
							else{s.push_back('g');i++;
	
							}

						}
						else{s.push_back('y');i++;
	
						}

					}
					else{i++;

						if(c[i]==0){i++;

							if(c[i]==0){i++;

								if(c[i]==0){s.push_back('b');i++;
	
								}
								else{i++;

									if(c[i]==0){i++;

										if(c[i]==0){s.push_back('c');i++;


										}
										else{i++;

											if(c[i]==0){i++;

												if(c[i]==0){s.push_back('w');i++;
	
												}
												else{s.push_back('q');i++;
	
												}
	
											}
											else{i++;

												if(c[i]==0){s.push_back('x');i++;
	
												}
												else{s.push_back('z');i++;
	
												}
											}
										}
	
									}
									else{s.push_back(' ');i++;
	
									}
	
								}

							}
							else{s.push_back('f');i++;
	
							}
	
						}
						else{s.push_back('j');i++;
	
						}

					}
	
				}
	
			}
			else{s.push_back('t');i++;
	
			}

		}

	}

   } //test

  std::ofstream fileout ("alphabetsrestored.txt"); //data
  if (fileout.is_open() )
  {
    char out;
    for(int i = 0; i<s.size(); ++i)
    {    
       out = s[i];
     fileout << out;  
    }
  fileout.close();
  }
  else std::cout << "File problem";

}
