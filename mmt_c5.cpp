#include <iostream>
#include <fstream>
#include <bitset>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

uint queue_max_size;
uint queue_min_size;

int res_summary;
int res_subtract;
char res_xor;

int first_value;

bool eof = false;
std::ifstream datafile;

std::mutex m;
std::condition_variable cv;
std::queue<int> queue;

void action_read(){
    int next_value;
    bool skip = false;
    //std::cerr << "Reader BEGIN" << std::endl;
    while( ! eof ) {
        if ( datafile >> next_value ) {
            //std::cerr << "r: " << next_value << std::endl;
            //std::cerr << "r: lock_guard call ... \n";
            std::lock_guard<std::mutex> lk{m};
            //std::cerr << "r:                 ... ok\n";

            if ( queue.size() > queue_max_size ) {
                skip = true;
            }
            if ( queue.size() > queue_min_size ) {
                skip = false;
            }
            if ( skip ) {
                continue;
            }

            queue.push(next_value);
        } else {
            //std::cerr << "r: EOF" << std::endl;
            eof = true;
        }
        //std::cerr << "r: notify_all sending ... " << queue.size() << "\n";
        cv.notify_all();
        //std::cerr << "r:                    ... send ok\n";
    }
    //std::cerr << "Reader END" << std::endl;
}

void action_sum(const int n){
    int next_value;
    std::string tab = "                ";
    //std::cerr << "Summator-" << n << " BEGIN" << std::endl;
    while( true ) {
        {
            //std::cerr << tab     << "S-" << n << ": unique_lock call ... \n";
            std::unique_lock<std::mutex> lk(m);
            //std::cerr << tab     << "S-" << n << ":                 ... ok\n";
            //std::cerr << tab     << "S-" << n << ": Waiting ... \n";
            while( queue.empty() && ! eof ) {
                //std::cerr << tab << "S-" << n << ":         ... 1.queue.empty()="<< queue.empty() <<", eof=" << eof << " \n";
                cv.wait(lk);
                //std::cerr << tab << "S-" << n << ":         ... 2.queue.empty()="<< queue.empty() <<", eof=" << eof << " \n";
            }
            //std::cerr << tab     << "S-" << n << ":         ... continue " << queue.size() << " eof" << eof << " empty:" << queue.empty() << "\n";
            if ( eof && queue.empty() ) {
                //std::cerr << tab << "S-" << n << ": exit" <<  std::endl;
                lk.unlock();
                break;
            }

            next_value = queue.front();
            queue.pop();

            //char first = res_xor;
            char second = (char)next_value;
            res_summary += next_value;
            res_xor = res_xor ^ second;

            lk.unlock();
        }


        // std::cerr << std::endl
        //     << tab << "S-" << n << ": xor 1 " << ((std::bitset<8>)first).to_string()   << " "       << (int)first   << std::endl
        //     << tab << "S-" << n << ": xor 2 " << ((std::bitset<8>)second).to_string()  << "    "    << (int)second  << std::endl
        //     << tab << "S-" << n << ": xor = " << ((std::bitset<8>)res_xor).to_string() << "       " << (int)res_xor << std::endl
        //     << std::endl;

        //std::cerr << tab << "S-" << n << ":" << res_summary << std::endl;
    }
    //std::cerr << "Summator-" << n << " END" << std::endl;
}

int main(){
    queue_max_size = 255;
    queue_max_size = 127;

    datafile.open("datafile.txt");
    if (datafile.is_open()) {
        std::cerr << "File 'datafile.txt' openned.\n";
    } else {
        std::cerr << "File 'datafile.txt' not exist or unaccessible...\n";
        return 1;
    }

    datafile >> first_value;
    res_summary = first_value;
    res_xor = first_value;

    std::thread thr_reader(action_read);
    std::thread thr_sum(action_sum, 1);
    std::thread thr_sum2(action_sum, 2);

    thr_reader.join();
    thr_sum.join();
    thr_sum2.join();


    res_subtract = first_value * 2 - res_summary;
    std::bitset<8> res_xor_bin(res_xor);

    std::cout << "Summary:     " << res_summary  << std::endl;
    std::cout << "Subtraction: " << res_subtract << std::endl;
    std::cout << "XOR:         " << res_xor_bin.to_string() << ", '"  << (int)res_xor << "'" << std::endl;

    datafile.close();
    return 0;
}
