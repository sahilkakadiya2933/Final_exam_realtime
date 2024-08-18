#!/bin/ksh

echo "Starting acceptance tests..."

# Launch the metronome with initial parameters
./metronome 120 2 4 &
METRONOME_PID=$!
sleep 3

# Check current metronome settings
echo "Checking current metronome settings"
cat /dev/local/metronome
echo ""

# Check metronome help information
echo "Checking metronome help information"
cat /dev/local/metronome-help
echo ""

# Modify the metronome configuration
echo "set 100 2 4 > /dev/local/metronome"
echo "set 100 2 4" > /dev/local/metronome
sleep 3
cat /dev/local/metronome
echo ""

# Modify the metronome configuration again
echo "set 200 5 4 > /dev/local/metronome"
echo "set 200 5 4" > /dev/local/metronome
sleep 3
cat /dev/local/metronome
echo ""

# Stop the metronome
echo "stop > /dev/local/metronome"
echo "stop" > /dev/local/metronome
sleep 1

# Restart the metronome
echo "start > /dev/local/metronome"
echo "start" > /dev/local/metronome
sleep 3
cat /dev/local/metronome
echo ""

# Pause the metronome for 3 seconds
echo "pause 3 > /dev/local/metronome"
echo "pause 3" > /dev/local/metronome
sleep 4

# Test pause with 10 seconds
echo "pause 10 > /dev/local/metronome"
echo "pause 10" > /dev/local/metronome
echo ""

# Test invalid command
echo "bogus > /dev/local/metronome"
echo "bogus" > /dev/local/metronome
echo ""

# Reset the metronome settings
echo "set 120 2 4 > /dev/local/metronome"
echo "set 120 2 4" > /dev/local/metronome
sleep 3
cat /dev/local/metronome
echo ""

# Display metronome help again
echo " cat /dev/local/metronome-help"
cat /dev/local/metronome-help
echo ""

# Attempt to write to the help device (should fail)
echo "Writes-Not-Allowed > /dev/local/metronome-help"
echo "Writes-Not-Allowed" > /dev/local/metronome-help
echo ""

# Terminate the metronome process
echo "quit > /dev/local/metronome"
echo "quit" > /dev/local/metronome
sleep 1

# Ensure the metronome has stopped running
if ps -p $METRONOME_PID > /dev/null; then
   echo "Metronome is still active."
else
   echo "Metronome has stopped."
fi

