Build deps
==========

*** Don't forget to unzip deps.zip in deps folder

D:\repo\cpp\kv_store>cmake -S ./deps/libzmq-master -B ./deps/build/zmq -DCMAKE_INSTALL_PREFIX=./deps/install -DCMAKE_POLICY_VERSION_MINIMUM=3.5
D:\repo\cpp\kv_store>cmake --build ./deps/build/zmq --config Release --target install

D:\repo\cpp\kv_store>cmake -S ./deps/quill-master -B ./deps/build/quill -DCMAKE_INSTALL_PREFIX=./deps/install
D:\repo\cpp\kv_store>cmake --build ./deps/build/quill --config Release --target install

Build source
============
D:\repo\cpp\kv_store>mkdir build && cd build

Build with STL
--------------

*** Don't forget to delete CMakeCache.txt file

D:\repo\cpp\kv_store\build>cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5

Or set DEASTL_BACKEND to OFF

D:\repo\cpp\kv_store\build>cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DEASTL_BACKEND=OFF

D:\repo\cpp\kv_store\build>cmake --build . --config Debug

D:\repo\cpp\kv_store\build>cmake --build . --config Release


Build with EASTL
----------------

D:\repo\cpp\kv_store\build>cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DEASTL_BACKEND=ON

D:\repo\cpp\kv_store\build>cmake --build . --config Debug

D:\repo\cpp\kv_store\build>cmake --build . --config Release