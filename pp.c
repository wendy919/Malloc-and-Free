void * mem_alloc(int size, int style)
{
	node_t *cur, *pre, *bcur, *bpre, *temp;
	header_t *header;
	void *ptr;
	int header_size;

	header_size = sizeof(header_t);
	pre = NULL;
	cur = head;
	bpre = NULL;
	bcur = head;

	int byte_size;
    void *ptr;
    
    byte_size = (size / 8 + 1) * 8;
    if(size % 8 == 0) {
        byte_size -= 8;
    }

	switch(style)
	{
		case M_BESTFIT:
		while(cur != NULL) {
        if(cur->size >= size && cur->size > wcur->size) {
            wcur = cur;
            wpre = pre;
        }
        pre = cur;
        cur = cur->next;
    	}
    	if(wcur->size < size ) {
        return NULL;
    	}
    	break;
		case M_WORSTFIT:
			while(cur != NULL) {
        if(cur->size >= size && cur->size > wcur->size) {
            wcur = cur;
            wpre = pre;
        }
        pre = cur;
        cur = cur->next;
    }
    if(wcur->size < size ) {
        return NULL;
    }
    break;
		case M_FIRSTFIT:
		while(cur->next != NULL && cur->size < size) {
        pre = cur;
        cur = cur->next;
    }
    
    if(cur->size < size) {
        return NULL;
    }
    break;
		default:
			m_error = E_BAD_ARGS;
			ptr = NULL;
			break;
	}

	if(ptr = NULL)
		m_error = E_NO_SPACE;

	if(cur == head) {
            head = (void *)head + size;
            head->next = cur->next;
            head->size = cur->size - size;
            header = (header_t *)cur;
            header->size = size - header_size;
            ptr = (void *)cur + header_size;
        }
        else {
            if(cur->size > size) {
                temp = cur;
                cur = (void *)temp + size;
                cur->next = temp->next;
                cur->size = temp->size - size;
                pre->next = cur;
                header = (header_t *)temp;
                header->size = size - header_size;
                ptr = (void *)header + header_size;
            }
            else {
                pre->next = cur->next;
                header = (header_t *)cur;
                header->size = size - header_size;
                ptr = (void *)cur + header_size;
            }
        }
    

	return ptr;
}