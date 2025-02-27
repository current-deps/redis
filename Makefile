.PHONY: all run clean test fmt

CLANG_FORMAT=$(shell echo "$${CLANG_FORMAT:-clang-format}")

all:
	pls b

redis-test:
	docker run --rm --net host --name test-redis -d redis --requirepass test

run: all
	./.debug/test

clean:
	rm -rf build

test: redis-test all
	sleep 1
	./.debug/test > tests/test.out
	docker kill test-redis
	diff tests/test.out tests/answer.test

fmt:
	${CLANG_FORMAT} -i src/*.cc src/*.h
