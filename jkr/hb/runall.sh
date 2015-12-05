echo Starting > /usr/test/results/out

export MAKE=/usr/test/test.sh
for q in 200 175 150 125 100 75 50 25 20 15 10 5 4 3 2 1
do
  /usr/test/schedswitch/schedswitch 1 ${q}
  sleep 5
  echo /usr/test/schedswitch/schedswitch 1 ${q} >> /usr/test/results/out
  worldstone 5
  cat time.master-dirty.worldstone.log >> /usr/test/results/out
  rm time.master-dirty.worldstone.log
done
/usr/test/schedswitch/schedswitch 3 3 
sleep 5
echo /usr/test/schedswitch/schedswitch 3 3  >> /usr/test/results/out

for q in 200 175 150 125 100 75 50 25 20 15 10 5 4 3 2 1
do
  /usr/test/schedswitch/schedswitch 1 ${q}
  sleep 5
  echo /usr/test/schedswitch/schedswitch 1 ${q} >> /usr/test/results/out
  worldstone 5
  cat time.master-dirty.worldstone.log >> /usr/test/results/out
  rm time.master-dirty.worldstone.log
done
cp /usr/test/results/out /usr/test/results/out.`date  '+%m%y%H%M'`


