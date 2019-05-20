            double stopTimeR1Up = 0; //currentTime + 2;
            double stopTimeR1Down = 0; // currentTime + 4;

            // Trigger moving R1
            double currentTime = GetSeconds(DateTime.Now);
            stopTimeR1Up = currentTime + 2;
            stopTimeR1Down = currentTime + 4;

            while (true) {
                currentTime = GetSeconds(DateTime.Now);
                if (stopTimeR1Up > currentTime) {
                    // R1 up
                    Console.WriteLine("R1Down off");
                    Thread.Sleep(100);
                    Console.WriteLine("R1Up on");
                }
                else {
                    if (stopTimeR1Down > currentTime) {
                        // R1 down
                        Console.WriteLine("R1Up off");
                        Thread.Sleep(100);
                        Console.WriteLine("R1DOwn on");
                    }
                    else {
                        Console.WriteLine("R1Up off");
                        Console.WriteLine("R1Down off");
                    }
                }
                //Thread.Sleep(500);
            }
