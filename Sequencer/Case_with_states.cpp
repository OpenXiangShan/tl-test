#include "Case_with_states.h"

namespace testcase_with_states {

    using namespace std;

    void Case_with_satets::read_test(std::string infile, bool re){
        string arr[arr_size];
        //read txt
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
        cout << "data size:" << n << endl;
        for (int i = 0; i < n; i++)
        {
            arr[i] = vec[i];
            // cout << vec[i] << endl;
        }

        // sort
        Mes_Entry mes_reset;
        mes_reset.opcode = reset_opcode;
        mes_reset.agentid = AGENT_BUS_TYPE_MAX;

        for (long i = 0; i < n; i+=base_long)
        {
            Mes_Entry mes;
            
            stringstream strValue;
            mes.agentid = (uint8_t)stoi(arr[i+n_agent_id]);
            mes.core_id = (uint8_t)stoi(arr[i+n_core_id]);
            mes.chnl = (uint8_t)stoi(arr[i+n_chnl]);
            mes.opcode = (uint8_t)stoi(arr[i+n_opcode]);
            mes.param = (uint8_t)stoi(arr[i+n_param]);

            for (int id = 0; id < 3; id++)
            {
                mes.state.self[id] = (uint8_t)stoi(arr[i+n_max + n_self + id*n_states]);
                mes.state.self_client[id][0] = (uint8_t)stoi(arr[i+n_max + n_self_client0 + id*n_states]);
                mes.state.self_client[id][1] = (uint8_t)stoi(arr[i+n_max + n_self_client1 + id*n_states]);
                mes.state.client[id][0] = (uint8_t)stoi(arr[i+n_max + n_client0 + id*n_states]);
                mes.state.client[id][1] = (uint8_t)stoi(arr[i+n_max + n_client1 + id*n_states]); 
            }

            for (uint8_t id = 0; id < n_cache_states; id++)
            {
                mes.begin_states[id] = (uint8_t)stoi(arr[i+n_max + 3*n_states + id]);
            }

            for (uint8_t id = 0; id < n_cache_states; id++)
            {
                mes.end_states[id] = (uint8_t)stoi(arr[i+n_max + 3*n_states + n_cache_states + id]);
            }

            uint64_t cycle = base_cycle + cycle_step*num;
            uint64_t cycle_reset = cycle + cycle_done;
            tc.insert(make_pair(cycle,mes));
            if(re)
                tc.insert(make_pair(cycle_reset,mes_reset));
            num++;

            //for test
            // HLOG(P_SW,"%d %d %d %d %d\n", tc[cycle].agentid, tc[cycle].core_id, tc[cycle].chnl, tc[cycle].opcode, tc[cycle].param);
            // for (uint8_t i = 0; i <3; i++)
            // {
            //     HLOG(P_SW,"%d %d %d %d %d\n", tc[cycle].state.self[3], tc[cycle].state.self_client[i][0], tc[cycle].state.self_client[i][1]
            //                                                             , tc[cycle].state.client[i][0], tc[cycle].state.client[i][1]);
            // }
            // for (uint8_t id = 0; id < n_cache_states; id++)
            // {
            //     HLOG(P_SW,"%d ", tc[cycle].begin_states[id]);
            // }
            // HLOG(P_SW,"\n");
            // for (uint8_t id = 0; id < n_cache_states; id++)
            // {
            //     HLOG(P_SW,"%d ", tc[cycle].end_states[id]);
            // }
            // HLOG(P_SW,"\n");
            
            
        }
        
        // for(std::set<check_point>::iterator it = point.begin(); it != point.end(); it++){
        //     HLOG(P_SW,"%d  %d  %d  %d  %d\n",it->mes[N_CH], it->mes[N_OP], it->mes[N_PA], it->mes[N_SCR], it->mes[N_CORE]);
        //     Tool::print(it->b_states);
        //     Tool::print(&it->e_states[0]);
        // }
        
        HLOG(P_SW,"Input num = %ld\n", tc.size());
        HLOG(P_SW,"%s Input Success!\n", infile.c_str());
    }

}