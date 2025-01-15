// Implementation of qIO::layeredOut

template<threeWayStrongComparable_t order_type>
void layeredOut::_deleteOrder(layeredOut &lo) noexcept{
	delete static_cast<order_t<order_type>*>(lo.order);
}

template<threeWayStrongComparable_t order_type>
layeredOut::layeredOut(
	uint32_t bgColor
) noexcept
:	state_v(0)
,	winSize_v(winSize_f())
,	shared({typeId<order_type>::value})
,	orderTypeId(typeId<order_type>::value)
,	createLayer_p(_createLayer<order_type>)
{
	static_assert(sizeof(rawListIter) == sizeof(typename std::list<order_type>::iterator));
	// test
	winSize_t a = winSize_f();
	size2d<size_t> b(a);

	b.w = 0;

	b = a;

	// test end


	std::cout<<winSize_v.w<<'x'<<winSize_v.h<<std::endl;
	if(!winSize_v.w){
		// state_v &= ~ 0x01;
		return;
	}

	try{
		oLayer[0] = oLayer[1] = oLayer[2]
		= std::vector<std::vector<charData>>(
			winSize_v.h,
			std::vector<charData>(
				winSize_v.w,
				charData()
			)
		);
	}catch(...){
		// state_v &= ~ 0x01;
		return;
	}

	order_t<order_type> *order = new(std::nothrow) order_t<order_type>(state_v, winSize_v);
	if(!order || (state_v & 0x8000'0000) ){
		// state_v &= ~ 0x01;
		return;
	}

	this->order = order;
	deleteOrder = _deleteOrder<order_type>;
	orderCreateLayer = order_t<order_type>::createLayer;

	std::setlocale(LC_ALL, "en_US.utf8");

	state_v |= 0x01;
}

template<threeWayStrongComparable_t T>
layeredOut::layer layeredOut::createLayer(winSize_t size, const T& order_v) noexcept{
	static constexpr const void* id_type_T = typeId<T>::value;
	assert(("type mismatch" && (orderTypeId == id_type_T)));

	return createLayer_p(this, size, &order_v);
}

template<threeWayStrongComparable_t T>
layeredOut::layer layeredOut::_createLayer(
	layeredOut *const inst_p,
	winSize_t size,
	const void *order_p
) noexcept
{
	const T& order_v = *static_cast<const T*>(order_p);

	try{
		std::optional<layeredOut::rawListIter> orderIter
		= inst_p->orderCreateLayer(inst_p->order, &order_v);

		if(!orderIter)
			return layer(0, key);

		inst_p->layerList.emplace_back(layerData{
			.visible = true,
			.data = std::vector<std::vector<charData>>(
				size.h,
				std::vector<charData>(
					size.w,
					charData()
				)
			),
			.pos = {0, 0},
			.orderIter = std::move(*orderIter)
		});
	}catch(...){
		return layer(0, key);
	}

	return layer(std::prev(inst_p->layerList.end()), &inst_p->shared, key);
}

// Implementation of qIO::layeredOut::order_t

template<threeWayStrongComparable_t T>
layeredOut::order_t<T>::order_t(state_t &state_v, const winSize_t &winSize) noexcept{
	try{
		oLayer[0] = oLayer[1] = oLayer[2]
		= std::vector<std::vector<T>>(
			winSize.h,
			std::vector<T>(winSize.w)
		);
	}catch(...){
		state_v |= 0x8000'0000;
		return;
	}
}

template<threeWayStrongComparable_t T>
std::optional<layeredOut::rawListIter> layeredOut::order_t<T>::createLayer(
	void *inst_p, const void *order_v
) noexcept
{
	order_t<T>& inst = *static_cast<order_t<T>*>(inst_p);

	try{
		inst.layerList.push_back(*static_cast<T*>(const_cast<void*>(order_v)));
	}catch(...){
		return std::nullopt;
	}

	return std::bit_cast<rawListIter>(std::prev(inst.layerList.end()));
}

// Implementation of qIO::layeredOut::layer

template<class T>
layeredOut::layerStream layeredOut::layer::operator<<(const T& a) noexcept{
	std::cout<<"layer::operator<<(const T&)"<<std::endl;
	layerStream ls(*this, cursor_v, key);
	ls << a;
	return ls;
//	return std::move(ls); // debug for when NRVO is disabled
}

// Implementation of qIO::layeredOut::layerStream

template<class T>
layeredOut::layerStream&& layeredOut::layerStream::operator<<(const T& a) noexcept{
	std::cout<<"layerStream::operator<<(const T&)"<<std::endl;
//	oss << a;
	layeredOut::layerStream& null = *reinterpret_cast<layeredOut::layerStream*>(reinterpret_cast<void*>(0));
	std::cout<<"this address : "<<this<<std::endl;
	std::cout<<"null address : "<<std::addressof(null)<<std::endl;
	return std::move(null);
	return std::move(*this);
}
