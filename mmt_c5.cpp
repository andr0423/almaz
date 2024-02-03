#include <iostream>
#include <fstream>
#include <bitset>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

//using namespace std;

const int queue_size = 2;

int res_summary;
int res_subtract;
char res_xor;

int first_value;
//int next_value;

bool eof = false;
std::ifstream datafile;

std::mutex m;
std::queue<int> queue;
std::condition_variable cv;
std::condition_variable cv2;

void action_read(){
    int next_value;
    std::cerr << "Reader BEGIN" << std::endl;
    while( ! eof ) {
        {
            std::cerr << "r: unique_lock call ... \n";
            std::unique_lock<std::mutex> lk(m);
            //std::cerr << "r: lock_guard call ... \n";
            //std::lock_guard<std::mutex> lk(m);
            std::cerr << "r:                 ... ok\n";

            while ( queue.size() > queue_size ) {
                std::cerr << "r: queue.size() to long ... = " << queue.size() << "\n";
                cv2.wait(lk, []{ return queue.size() <= queue_size ; } );
                std::cerr << "r: queue.size() reduced ... = " << queue.size() << "\n";
            }
            if ( datafile >> next_value ) {
                std::cerr << "r: " << next_value << std::endl;
                queue.push(next_value);
            } else {
                std::cerr << "r: EOF" << std::endl;
                eof = true;
                //queue.push(0);
                //cv.notify_all();
                //break;
            }
            //std::cerr << "r: unlock call ... " << queue.size() << "\n";
            lk.unlock();
            //std::cerr << "r:             ... ok \n";
            //std::cerr << "r: lock_guard FREE ... queue.size=" << queue.size() << "\n";
        }
        std::cerr << "r: notify_all sending ... " << queue.size() << "\n";
        cv.notify_all();
        std::cerr << "r:                    ... send ok\n";
    }
    std::cerr << "Reader END" << std::endl;
}

void action_sum(){
    std::string tab = "                ";
    std::cerr << "Summator BEGIN" << std::endl;
    while( true ) {
        {
            std::cerr << tab     << "S: unique_lock call ... \n";
            std::unique_lock<std::mutex> lk(m);
            std::cerr << tab     << "S:                 ... ok\n";
            std::cerr << tab     << "S: Waiting ... \n";
            //cv.wait( lk, []{ return !queue.empty() || eof ; } );
            while( queue.empty() && ! eof ) {
                std::cerr << tab << "S:         ... 1.queue.empty()="<< queue.empty() <<", eof=" << eof << " \n";
                cv.wait(lk);
                std::cerr << tab << "S:         ... 2.queue.empty()="<< queue.empty() <<", eof=" << eof << " \n";
            }
            std::cerr << tab     << "S:         ... continue " << eof << " " << queue.empty() << "\n";
            if ( eof && queue.empty() ) {
                std::cerr << tab << "S: exit" <<  std::endl;
                lk.unlock();
                break;
            }

            char first = res_xor;
            char second = (char)queue.front(); 

            res_summary += queue.front();
            res_xor = res_xor ^ (char)queue.front();
            queue.pop();
            cv2.notify_all();

            std::cerr << "S: xor 1 " << ((std::bitset<8>)first).to_string()   << " " << (int)first   << std::endl
                      << "S: xor 2 " << ((std::bitset<8>)second).to_string()  << "    " << (int)second  << std::endl
                      << "S: xor = " << ((std::bitset<8>)res_xor).to_string() << "       " << (int)res_xor << std::endl
                      << std::endl;

            lk.unlock();
        }

        std::cerr << tab << "S:" << res_summary << std::endl;

    }
    std::cerr << "Summator END" << std::endl;
}

// void action_xor(){
//     while(!eof) {
//         g_mut_xor.lock();
//         res_xor ^= (char)next_value;
//         //g_mut_xor.unlock();
//         g_mut_xor_reader.unlock();
//     }
// }

int main(){

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
    std::cerr << "First:" << first_value << ", sum:" << res_summary << ", xor:"  << (int)res_xor << std::endl;

    std::thread thr_reader(action_read);
    std::thread thr_sum(action_sum);
    //std::thread thr_xor(action_xor);

    thr_reader.join();
    thr_sum.join();
    //thr_xor.join();

    res_subtract = first_value * 2 - res_summary;
    std::bitset<8> res_xor_bin(res_xor);

    //std::cerr << "Last:" << next_value << std::endl;

    std::cout << "Summary:     " << res_summary  << std::endl;
    std::cout << "Subtraction: " << res_subtract << std::endl;
    std::cout << "XOR:         " << res_xor_bin.to_string() << ", '"  << (int)res_xor << "'" << std::endl;

    datafile.close();
    return 0;
}
