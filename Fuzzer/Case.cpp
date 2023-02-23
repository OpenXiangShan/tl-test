//
// Created by dxy on 2023/2/15.
//

#include "Case.h"

namespace testcase {

    Message::Message()
    {
        mapping.clear();
    }

    Message::~Message()
    {

    }

    void Message::clear(void) {
        mapping.clear();
    }

    void Message::update(const uint64_t& key, const Mes_Entry& mes){
        if(mapping.count(key) != 0){
            mapping[key] = mes;
        }else{
            mapping.insert(std::make_pair(key,mes));
        }
    }

    bool Message::haskey(const uint64_t& key) {
        return mapping.count(key) > 0;
    }


    Mes_Entry Message::query(const uint64_t& key) {//询问
        if (mapping.count(key) > 0) {
            return mapping[key];
        } else {
            tlc_assert(false, "Key no found!");
        }
    }


    void read_file::read_test(Message agent_message[N_TESTCASE_AGENT])
    {
        using namespace std;

        string arr[10000] = { };
        //read txt
        string infile = "test.txt";
        vector<string> vec;
        ifstream input(infile);
        if (input)
        {
            string word;
            while (input >> word) {
            vec.push_back(word); // Insert the element at the end of the vector
            }
        }
        else {
            cout << "Could not open the file " << endl;
            return;
        }//Output Could not open file if not inserted successfully
        input.close();

        //save data
        int n = vec.size(); 
        // cout << "data size：" << n << endl;
        for (int i = 0; i < n; i++)
        {
            arr[i] = vec[i];
            // cout << vec[i] << endl;
        }
        //sort data
        for (int i = 0; i < n; i += n_last) {
            stringstream strValue;
            testcase::Mes_Entry mes;
            uint64_t cycle;
            mes.test_case = stoi(arr[i+n_case]);
            strValue<<arr[i+n_cycle];
            strValue>>cycle;
            mes.id = stoi(arr[i+n_id]);
            mes.link = stoi(arr[i+n_link]);
            mes.opcode = (uint8_t)stoi(arr[i+n_op]);
            mes.param = (uint8_t)stoi(arr[i+n_param]);
            mes.addr = (paddr_t)stol(arr[i+n_addr], NULL, 0);
            mes.user = (uint8_t)stoi(arr[i+n_user]);


            if(mes.id == -1){
                agent_message[0].update(cycle,mes);//Recorded in mes[0] opcode == reset_opcode means reset the sys
            }
            else if(mes.id < N_TESTCASE_AGENT){
                agent_message[mes.id].update(cycle,mes);
            }
        }
        printf("Input Case Success!\n");

    }


}
