add_library(
        CacheModel base_cache_model.h 
        FakeL1/fake_l1.cpp FakeL1/fake_l1.h
        FakePTW/fake_ptw.cpp FakePTW/fake_ptw.h
        tl_c_agent.cpp tl_c_agent.h
        tl_ul_agent.cpp tl_ul_agent.h
        tl_base_agent.h
        )