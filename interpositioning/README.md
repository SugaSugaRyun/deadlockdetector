# Build and Run

    $gcc -shared -fPIC -o mylock mylock.c -ldl  
    $LD_PRELOAD="./mylock.so"
    $./target