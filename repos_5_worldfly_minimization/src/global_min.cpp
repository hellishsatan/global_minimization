#include "global_min.hpp"

#include <array>

#include "thread_pool.h"

/*
std::vector<std::pair<Real, Vector>>
calc_f_with_threads(Function f, const std::vector<Vector>& inData)
{
    // Создаем вектор под ответ:
    std::vector<std::pair<Real, Vector>> outData(inData.size());

    // Количество ядер:
    uint32_t nCores = std::max(1u, std::thread::hardware_concurrency());

    // Вектор из тредов:
    std::vector<std::thread> t;

    // Мьютексы на чтение и запись:
    std::mutex inRead, outWrite;

    uint32_t globalIndex = 0;

    // Создаем столько тредов, сколько ядер:
    for (uint32_t i = 0; i < nCores; i++)
    {
        t.push_back(std::thread([&]
        {
            while (1)
            {
                inRead.lock();
                uint32_t i = globalIndex;
                if (globalIndex >= inData.size())
                {
                    inRead.unlock();
                    return;
                }

                auto x = inData[i];
                globalIndex++;
                inRead.unlock();

                auto res = f(x);

                outWrite.lock();
                outData[i] = {res, x};
                outWrite.unlock();
            }
            return;
        }));
    }

    // Присоединяем все треды к материнскому треду, гарантируя то, что все они будут выполнены
    for (uint32_t i = 0; i < nCores; ++i)
    {
        t[i].join();
    }

    assert(globalIndex == inData.size());

    return outData;
}
*/

std::vector<std::pair<Real, Vector>>
find_local_mins_with_threads(Function f, const StopCondition& stop_condition,
                             const std::vector<std::pair<Real, Vector>>& inData) {
    // Создаем вектор под ответ
    std::vector<std::pair<Real, Vector>> outData(inData.size());

    // Количество потоков
    const uint32_t cores = ThreadPool::GetHardwareCoresAmount();

    // Создаем пул потоков
    ThreadPool thread_pool{cores};

    // Массив функций минимизации
    static const std::array<std::function<IterationData(Function, Vector, const StopCondition&)>, 6> min_funcs{
        &bfgs, &hessian_free, &slow_hessian_free, &nesterov, &dfp, &powell
    };

    // Мьютекс на запись
    pthread_mutex_t outWrite;
    pthread_mutex_init(&outWrite, nullptr);

    for (size_t i = 0; i < inData.size(); ++i)
    {
        const auto& data = inData[i];

        // Записываем начальное значение
        outData[i] = data;

        for (const auto& min_func : min_funcs)
        {
            thread_pool.Add([&, data, i]()
            {
                const auto iter_data = min_func(f, data.second, stop_condition);
                const auto x_curr = iter_data.x_curr;
                const auto f_curr = iter_data.f_curr;

                const auto pair = std::make_pair(f_curr, x_curr);

                // Записываем ответ
                pthread_mutex_lock(&outWrite);

                // Заменяемым минимальным
                outData[i] = std::min(outData[i], pair);

                pthread_mutex_unlock(&outWrite);
            });
        }
    }

    thread_pool.CreateThreads();
    thread_pool.WaitForAll();

    return outData;
}


std::vector<std::pair<Real, Vector>>
find_absmin(Function f, const StopCondition& stop_condition, uint32_t dim, uint32_t nBestPoints, uint32_t nAllPoints,
            Vector min, Vector max)
{
    /*// Несколько проверок на входные данные:
    assert(dim > 0u && dim == min.size() && dim == max.size());
    assert(nBestPoints <= nAllPoints && nBestPoints > 0u);
    for (uint32_t i = 0; i < dim; ++i)
    {
        assert(min[i] <= max[i]);
    }

    // Объект-генератор сетки:
    SobolSeqGenerator net;
    //net.Init(nAllPoints, dim, "D:\\Programs\\VS\\worldfly_minimization\\WorldflyMinimization\\bin\\tests\\new-joe-kuo-6.21201.txt");
    net.Init(nAllPoints, dim,
             "D:\\Programs\\VS\\worldfly_minimization\\WorldflyMinimization\\bin\\tests\\new-joe-kuo-6.21201.txt");

    // ----- Первый этап: вычисление значений функции в узлах сетки с отбором точек-кандидатов -----

    // Лимит на группу из одновременно обрабатываемых точек:
    const uint32_t GROUP_SIZE = 1024u;

    // Формирование списка лучших кандидатов
    std::set<std::pair<Real, Vector>> candidates;

    // Сначала складываем точки группами по GROUP_SIZE:
    std::vector<Vector> group;
    for (uint32_t i = 0; i < nAllPoints; ++i)
    {
        // Получение текущего узла сетки:
        const auto net_point = net.GeneratePoint().coordinate;
        // Преобразование узла к точке в ограниченной min и max области:
        Vector curr(dim);
        for (uint32_t i = 0; i < dim; ++i)
        {
            curr[i] = net_point[i] * (max[i] - min[i]) + min[i];
        }
        if (i == nAllPoints - 1 || group.size() == GROUP_SIZE)
        {
            // Запускаем многопоточное вычисление значений во всех точках
            for (auto& it : calc_f_with_threads(f, group))
            {
                candidates.insert(it);
                if (candidates.size() > nBestPoints)
                {
                    candidates.erase(std::prev(candidates.end()));
                }
            }
            group.clear();
        }
        else
        {
            group.push_back(curr);
        }
    }

    // ----- Второй этап: запуск алгоритмов поиска локального минимума из выбранных точек -----
    // Подготовка (перекладываем точки из set в vector - возможен рост скорости при последовательном размещении в памяти точек):
    std::vector<std::pair<Real, Vector>> temp;
    for (auto& it : candidates)
    {
        temp.push_back(it);
    }

    // Многопоточная обработка кандидатов:
    auto answer = find_local_mins_with_threads(f, stop_condition, temp);

    // Итоговая сортировка всех найденных точек по неубыванию значения функции в них:
    std::sort(answer.begin(), answer.end());
    
    return answer;*/
    return {};
}
// tms-сети возвращают нам вектора, компоненты которых принадлежат [0,1)
void __tmp_tms_result_imitation(unsigned int n, unsigned int numOfDimension){
	std::ofstream output_file;
	output_file.open("tms-result-imitation.txt", std::ios::app);
	for (unsigned int i = 0; i < n; i++){
			for (unsigned int j = 0; j < numOfDimension; j++){
				if (j != numOfDimension -1){
					output_file << uniform(0,1) << " ";
				} else {
					output_file << uniform(0,1);
				}
			}
			if (i != n-1){
				output_file << std::endl;
			}
		}
	output_file.close();
}

// Поток потребитель
void* my_calc_f_with_threads(void* args) {
    consumerArgs* consArg = static_cast<consumerArgs*>(args);

    while (true) {
        pthread_mutex_lock(&consArg->queueMutex);

        // Если элементов больше не будет, то завершаем работу
        if (consArg->eof && consArg->queueOfPoints.empty()) {
            pthread_mutex_unlock(&consArg->queueMutex);
            break;
        }
        // Если остается мало элементов, сигналим поставщику
        if (consArg->queueOfPoints.size() < 256)
            pthread_cond_signal(&consArg->canProduce);

        // Ждем пока появятся элементы в очереди
        while (consArg->queueOfPoints.empty()) {
            pthread_cond_wait(&consArg->canConsume, &consArg->queueMutex);

            // Снова проверяем, будут ли еще элементы
            if (consArg->eof && consArg->queueOfPoints.empty()) {
                pthread_mutex_unlock(&consArg->queueMutex);
                pthread_exit(nullptr);
                return nullptr;
            }
        }

        // Берем элемент из очереди
        Vector point = consArg->queueOfPoints.front();
        consArg->queueOfPoints.pop();


        pthread_mutex_unlock(&consArg->queueMutex);

        // Выполняем функцию
        auto res = consArg->f(point);

        // Сохраняем результат

        pthread_mutex_lock(&consArg->writeMutex);

        consArg->candidates.emplace(res, point);
        if (consArg->candidates.size() > consArg->nBestPoints){
        	consArg->candidates.erase(std::prev(consArg->candidates.end()));
        }

        pthread_mutex_unlock(&consArg->writeMutex);
    }

    pthread_exit(nullptr);

    return nullptr;
}

// Функция для треда-поставщика
void* add_points_to_queue(void *args){
	producerArgs *ptr_producer_args = (producerArgs *)args;
	std::queue<Vector> tmpQueue;

	// Размер нашей очереди
	unsigned int tmpQueueSize = 2048;
	bool exit = false;
	// Открываем файл
	std::ifstream file;
	file.open("tms-result-imitation.txt");
	std::string str;
	while (!exit){
		// счётчик количества элементов
		unsigned int count = 0;
		while (true){
			// Если строка еще есть
			if (std::getline(file, str)) {
				// Сплитим строку по пробелам, получаем вектор строк results
				std::istringstream iss(str);
				std::vector<std::string> results((std::istream_iterator<std::string>(iss)),
				std::istream_iterator<std::string>());
				Vector v(results.size());
				for (unsigned int i = 0; i < results.size(); i++){
					// std::stold приводит строку к long double
					// Приводим точку на единичном s-мерном кубе к точке в s-мерном прямоугольном параллелепипеде
					v[i] = std::stold(results[i]) * (ptr_producer_args->max[i] - ptr_producer_args->min[i]) + ptr_producer_args->min[i];
				}
				tmpQueue.emplace(v);
				count++;

				if (count >= tmpQueueSize){
					// Захватываем мьютекс
					pthread_mutex_lock(&ptr_producer_args->queueMutex);
					// Ждём когда нам посигналят треды-потребители добавить еще элементов
					// При вызове wait мьютекс освободится, после ожидания снова захватится, когда ему посигналят
					pthread_cond_wait(&ptr_producer_args->canProduce, &ptr_producer_args->queueMutex);

					// ВОЛШЕБНАЯ ОПЕРАЦИЯ MOVE СЕРЕЖА НАПИШИ ЕЕ САМ ПЛЕС Я ЗА НЕЕ НЕ ШАРЮ

					// Сигналим тредам-потребителям, что в очереди появились элементы
					//
					pthread_cond_broadcast(&ptr_producer_args->canConsume);

					count = 0;

					break;

				}

			} else {
				// Выход из цикла, элементов больше не будет
				exit = true;
				// Захватываем мьютекс, чтоб сообщить тредам-потребителям что файл кончился
				pthread_mutex_lock(&ptr_producer_args->queueMutex);
				ptr_producer_args->eof = true;

				// МУВАЕМ ОСТАВШИЕСЯ ЭЛЕМЕНТЫ В ОЧЕРЕДЬ ЕСЛИ ЕСТЬ. Я ЗА МУВЫ НЕ ШАРЮ СЕРЕЖА

				pthread_mutex_unlock(&ptr_producer_args->queueMutex);
				break;
			}
		}

	}
	file.close();
	return nullptr;
}

void
my_find_absmin(Function f, const StopCondition& stop_condition, uint32_t dim, uint32_t nBestPoints, uint32_t nAllPoints, Vector min, Vector max) {
	// Несколько проверок на входные данные:
	assert(dim > 0u && dim == min.size() && dim == max.size());
	assert(nBestPoints <= nAllPoints && nBestPoints > 0u);
	for (uint32_t i = 0; i < dim; ++i) {
		assert(min[i] <= max[i]);
	}

	// Тред-поставщик
	pthread_mutex_t queueMutex;
	pthread_mutex_init(&queueMutex, nullptr);

	pthread_cond_t queueCondAddMore;
	pthread_cond_init(&queueCondAddMore, nullptr);

	pthread_cond_t queueCondEndOfFile;
	pthread_cond_init(&queueCondEndOfFile, nullptr);

	// Формирование списка лучших кандидатов
	std::set<std::pair<Real, Vector>> candidates;

	pthread_t provider;
	std::queue<Vector> queueOfPoints;
	volatile bool eof = false;
	producerArgs provider_args{queueOfPoints, queueMutex, queueCondAddMore, queueCondEndOfFile, min, max, eof};
	pthread_create(&provider, nullptr, &add_points_to_queue, (void*)&provider_args);

//	// ----- Первый этап: вычисление значений функции в узлах сетки с отбором точек-кандидатов -----
//
//	// Лимит на группу из одновременно обрабатываемых точек:
//	const uint32_t GROUP_SIZE = 1024u;
//
//
//
//	// Сначала складываем точки группами по GROUP_SIZE:
//	std::vector<Vector> group;
//	for (uint32_t i = 0; i < nAllPoints; ++i) {
//		// Получение текущего узла сетки:
//		const auto net_point = net.GeneratePoint().coordinate;
//		// Преобразование узла к точке в ограниченной min и max области:
//		Vector curr(dim);
//		for (uint32_t i = 0; i < dim; ++i) {
//			curr[i] = net_point[i] * (max[i] - min[i]) + min[i];
//		}
//		if (i == nAllPoints - 1 || group.size() == GROUP_SIZE) {
//			// Запускаем многопоточное вычисление значений во всех точках
//			for (auto& it : calc_f_with_threads(f, group)) {
//				candidates.insert(it);
//				if (candidates.size() > nBestPoints) {
//					candidates.erase(std::prev(candidates.end()));
//				}
//			}
//			group.clear();
//		} else {
//			group.push_back(curr);
//		}
//	}
//
//	// ----- Второй этап: запуск алгоритмов поиска локального минимума из выбранных точек -----
//	// Подготовка (перекладываем точки из set в vector - возможен рост скорости при последовательном размещении в памяти точек):
//	std::vector<std::pair<Real, Vector>> temp;
//	for (auto & it : candidates) {
//		temp.push_back(it);
//	}
//
//	// Многопоточная обработка кандидатов:
//	auto answer = find_local_mins_with_threads(f, stop_condition, temp);
//
//	// Итоговая сортировка всех найденных точек по неубыванию значения функции в них:
//	std::sort(answer.begin(), answer.end());

	pthread_join(provider, nullptr);
	return;
}
