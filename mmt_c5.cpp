#include <iostream>
#include <fstream>
#include <bitset>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

bool eof = false;
std::ifstream datafile;
std::mutex m;
std::condition_variable cv;
std::queue<int> queue;

void action_read(const uint queue_max_size, const uint queue_min_size){
    int next_value;
    bool skip = false;
    while( ! eof ) {

        std::lock_guard<std::mutex> lk(m);

        uint qs = queue.size();
        if ( qs > queue_max_size ) {
            skip = true;
        }
        if ( skip ) {
            if ( qs < queue_min_size ) {
                skip = false;
            }
            cv.notify_all();
            continue;
        }

        if ( datafile >> next_value ) {
            queue.push(next_value);
        } else {
            eof = true;
        }
        cv.notify_all();

    }
}

void action_sum( const int n , int & sm , char & xr ) {
    int sum_th = 0;
    char xor_th = 0;
    while( true ) {
        
        std::unique_lock<std::mutex> lk(m);

        while( queue.empty() && ! eof ) {
            cv.wait(lk);
        }
        if ( eof && queue.empty() ) {
            lk.unlock();
            break;
        }

        int next_value = queue.front();
        queue.pop();

        sum_th += next_value;
        xor_th ^= (char)next_value;;

        lk.unlock();

    }
    sm = sum_th;
    xr = xor_th;
}

int main(){
    uint queue_max_size = 255;
    uint queue_min_size = 127;

    int  res_summary;
    int  res_subtract;
    char res_xor_all;

    int first_value;

    int  res_sum_1 = 0, res_sum_2 = 0;
    char res_xor_1 = 0, res_xor_2 = 0;

    datafile.open("datafile.txt");
    if (datafile.is_open()) {
        std::cerr << "File 'datafile.txt' openned.\n";
    } else {
        std::cerr << "File 'datafile.txt' not exist or unaccessible...\n";
        return 1;
    }

    datafile >> first_value;

    std::thread thr_reader( action_read, queue_max_size, queue_min_size );
    std::thread thr_sum   ( action_sum, 1, std::ref(res_sum_1), std::ref(res_xor_1) );
    std::thread thr_sum2  ( action_sum, 2, std::ref(res_sum_2), std::ref(res_xor_2) );

    thr_reader.join();
    thr_sum.join();
    thr_sum2.join();

    int thread_sum = res_sum_1 + res_sum_2;

    res_summary  = first_value + thread_sum;
    res_subtract = first_value - thread_sum;
    res_xor_all  = (char)first_value ^ res_xor_1 ^ res_xor_2;

    std::bitset<8> res_xor_bin2(res_xor_all);
    std::cout << "Summary:     " << res_summary  << std::endl;
    std::cout << "Subtraction: " << res_subtract << std::endl;
    std::cout << "XOR:         " << res_xor_bin2.to_string() << ", '"  << (int)res_xor_all << "'" << std::endl;

    datafile.close();
    return 0;
}
