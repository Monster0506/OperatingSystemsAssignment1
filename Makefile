CXX = g++
CXXFLAGS = -Wall -lrt

# Targets
all: producer consumer

producer: producer.cpp shared.hpp
	$(CXX) $(CXXFLAGS) producer.cpp -o producer

consumer: consumer.cpp shared.hpp
	$(CXX) $(CXXFLAGS) consumer.cpp -o consumer

run: all
	./producer & ./consumer &

clean:
	rm -f producer consumer
